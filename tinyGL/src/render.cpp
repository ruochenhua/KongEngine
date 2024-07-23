#include "render.h"

#include "Camera.h"
#include "Engine.h"
#include "model.h"
#include "tgaimage.h"
#include "message.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

int CRender::Init()
{
	render_window = Engine::GetRenderWindow();
	InitCamera();

	// ��ʼ����պ�
	m_SkyBox.Init();
	//init show map
	glGenFramebuffers(1, &m_FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);

	// ? �����ͼ���
	glGenTextures(1, &m_DepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture, 0);
	// ���buffer
	glDrawBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return -1;

	// ����shader�����벢������Ӧ�ĳ���
	// m_ShadowMapProgramID = LoadShaders("../../../../resource/shader/shadowmap_vert.shader",
	// 	"../../../../resource/shader/shadowmap_frag.shader");

	m_DepthMatrixID = glGetUniformLocation(m_ShadowMapProgramID, "depth_mvp");
	return 0;
}

int CRender::InitCamera()
{
	mainCamera = new CCamera(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	mainCamera->InitControl();
	return 0;
}

int CRender::Update(double delta)
{
	UpdateLightDir(delta);
	//update camera
	mainCamera->Update(delta);		
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	glViewport(0, 0, 1024, 1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	// for (auto& render_info : m_vRenderInfo)
	// {
	// 	RenderShadowMap(render_info);
	// }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//opengl32.dll	
	glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	RenderSkyBox();
	for (auto& render_info : m_vRenderInfo)
	{
		RenderModel(render_info);
	}
	
	return 1;
}

void CRender::PostUpdate()
{
	// Swap buffers
	glfwSwapBuffers(render_window);
	glfwPollEvents();
}

void CRender::AddRenderInfo(SRenderInfo render_info)
{	
	m_vRenderInfo.push_back(render_info);
}

void CRender::RenderSceneObject(shared_ptr<CRenderObj> render_obj)
{
	const SRenderInfo& render_info = render_obj->GetRenderInfo();
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(render_info.program_id);
	glBindVertexArray(render_info.vertex_array_id);	// ��VAO
	
	mat4 Model = render_obj->GetModelMatrix();
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrix() * Model; //
	GLuint matrix_id = glGetUniformLocation(render_info.program_id, "MVP");
	glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

	GLuint light_shininess_id = glGetUniformLocation(render_info.program_id, "shininess");
	glUniform1f(light_shininess_id, render_info.material.shininess);

	GLuint cam_pos_id = glGetUniformLocation(render_info.program_id, "cam_pos");
	glUniform3fv(cam_pos_id, 1, &mainCamera->GetPosition()[0]);

	GLuint light_dir_id = glGetUniformLocation(render_info.program_id, "light_dir");
	glUniform3fv(light_dir_id, 1, &m_LightDir[0]);
	
	GLuint light_color_id = glGetUniformLocation(render_info.program_id, "light_color");
	glUniform3fv(light_color_id, 1, &m_LightColor[0]);

	//use shadow map
	mat4 bias_mat(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	mat4 depth_bias_mvp = bias_mat * m_DepthMVP;

	GLuint depth_bias_id = glGetUniformLocation(render_info.program_id, "depth_bias_mvp");
	glUniformMatrix4fv(depth_bias_id, 1, GL_FALSE, &depth_bias_mvp[0][0]);

	GLuint model_mat_id = glGetUniformLocation(render_info.program_id, "normal_model_mat");

	/*
	法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
	Normal = mat3(transpose(inverse(model))) * aNormal;
	 */
	mat3 normal_model_mat = transpose(inverse(Model));
	glUniformMatrix3fv(model_mat_id, 1, GL_FALSE, &normal_model_mat[0][0]);
	
	if(render_info.diffuse_tex_id)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, render_info.diffuse_tex_id);
	}

	if(render_info.specular_map_tex_id)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, render_info.specular_map_tex_id);
	}

	// Draw the triangle !
	// if no index, use draw array
	if(render_info.index_buffer == GL_NONE)
	{
		glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); // Starting from vertex 0; 3 vertices total -> 1 triangle	
	}
	else
	{		
		glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
	}
	
	glBindVertexArray(GL_NONE);	// ���VAO
}


void CRender::RenderSkyBox()
{
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrixNoTranslate(); //
	m_SkyBox.Render(mvp);
}

void CRender::RenderShadowMap(const SRenderInfo& render_info)
{		
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

	// ʹ����Ӱ��ͼ��shader
	glUseProgram(m_ShadowMapProgramID);	

	// Compute the MVP matrix from the light's point of view
	//1024.0f / 768.0f, 0.1f, 500.0f
	mat4 depth_proj_mat = ortho<float>(-10, 10, -10, 10, -10, 20);
	mat4 depth_view_mat = lookAt(m_LightDir+m_LightPos, m_LightPos, glm::vec3(0, 1, 0));

	mat4 depth_model_mat = mat4(1.0);
	m_DepthMVP = depth_proj_mat * depth_view_mat * depth_model_mat;

	// in the "MVP" uniform
	glUniformMatrix4fv(m_DepthMatrixID, 1, GL_FALSE, &m_DepthMVP[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, render_info.vertex_buffer);
	glVertexAttribPointer(
		0,  // The attribute we want to configure
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);	

	glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle	

	glDisableVertexAttribArray(0);
}

void CRender::RenderModel(const SRenderInfo& render_info) const
{	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(render_info.program_id);
	glBindVertexArray(render_info.vertex_array_id);	// ��VAO
	
	mat4 Model = mat4(1.0f);
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrix() * Model; //
	GLuint matrix_id = glGetUniformLocation(render_info.program_id, "MVP");
	glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

	GLuint light_shininess_id = glGetUniformLocation(render_info.program_id, "shininess");
	glUniform1f(light_shininess_id, render_info.material.shininess);

	GLuint cam_pos_id = glGetUniformLocation(render_info.program_id, "cam_pos");
	glUniform3fv(cam_pos_id, 1, &mainCamera->GetPosition()[0]);

	GLuint light_dir_id = glGetUniformLocation(render_info.program_id, "light_dir");
	glUniform3fv(light_dir_id, 1, &m_LightDir[0]);
	
	GLuint light_color_id = glGetUniformLocation(render_info.program_id, "light_color");
	glUniform3fv(light_color_id, 1, &m_LightColor[0]);

	//use shadow map
	mat4 bias_mat(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	mat4 depth_bias_mvp = bias_mat * m_DepthMVP;

	GLuint depth_bias_id = glGetUniformLocation(render_info.program_id, "depth_bias_mvp");
	glUniformMatrix4fv(depth_bias_id, 1, GL_FALSE, &depth_bias_mvp[0][0]);

	GLuint model_mat_id = glGetUniformLocation(render_info.program_id, "normal_model_mat");

	/*
	法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
	Normal = mat3(transpose(inverse(model))) * aNormal;
	 */
	mat3 normal_model_mat = transpose(inverse(Model));
	glUniformMatrix3fv(model_mat_id, 1, GL_FALSE, &normal_model_mat[0][0]);
	
	if(render_info.diffuse_tex_id)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, render_info.diffuse_tex_id);
	}

	if(render_info.specular_map_tex_id)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, render_info.specular_map_tex_id);
	}

	// Draw the triangle !
	// if no index, use draw array
	if(render_info.index_buffer == GL_NONE)
	{
		glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); // Starting from vertex 0; 3 vertices total -> 1 triangle	
	}
	else
	{		
		glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
	}
	
	glBindVertexArray(GL_NONE);	// ���VAO
}

void CRender::UpdateLightDir(float delta)
{
	// ���ձ仯
	double light_speed = 30.0;

	light_yaw += light_speed*delta;
	
	vec3 front;
	front.x = cos(glm::radians(light_yaw)) * cos(glm::radians(light_pitch));
	front.y = sin(glm::radians(light_pitch));
	front.z = sin(glm::radians(light_yaw)) * cos(glm::radians(light_pitch));

	m_LightDir = normalize(front);
	// printf("--- light dir %f %f %f\n", m_LightDir.x, m_LightDir.y, m_LightDir.z);
}

