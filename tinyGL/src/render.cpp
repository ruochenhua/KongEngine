#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Camera.h"
#include "Engine.h"
#include "light.h"
#include "model.h"
#include "tgaimage.h"
#include "message.h"
#include "shader.h"
#include "stb_image.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

int CRender::Init()
{
	render_window = Engine::GetRenderWindow();
	InitCamera();

	// m_SkyBox.Init();
	//init show map
	//glGenFramebuffers(1, &m_FrameBuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	
	glGenTextures(1, &m_DepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture, 0);
	// buffer
	// glDrawBuffer(GL_NONE);
	// if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	// 	return -1;
	//
	// m_ShadowMapProgramID = LoadShaders("../../../../resource/shader/shadowmap_vert.shader",
	// 	"../../../../resource/shader/shadowmap_frag.shader");

	//m_DepthMatrixID = glGetUniformLocation(m_ShadowMapProgramID, "depth_mvp");

	// load null texture
	string null_tex_path = RESOURCE_PATH + "Engine/null_texture.png";
	null_tex_id = CRender::LoadTexture(null_tex_path);
	return 0;
}

int CRender::InitCamera()
{
	mainCamera = new CCamera(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	mainCamera->InitControl();
	return 0;
}

void CRender::InitLights(const vector<shared_ptr<Light>>& lights)
{
	scene_lights = lights;
}

int CRender::Update(double delta)
{
	//UpdateLightDir(delta);
	//update camera
	mainCamera->Update(delta);		
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	// glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
	
	//RenderSkyBox();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	return 1;
}

void CRender::PostUpdate()
{
	
	// Swap buffers
	glfwSwapBuffers(render_window);
	glfwPollEvents();
}

void CRender::RenderSceneObject(shared_ptr<CRenderObj> render_obj)
{
	GLuint shader_id = render_obj->GetShaderId();

	glUseProgram(shader_id);
	for(auto& mesh : render_obj->mesh_list)
	{
		const SRenderInfo& render_info = mesh.GetRenderInfo();
		glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
	
		mat4 model_mat = render_obj->GetModelMatrix();
		mat4 view_mat = mainCamera->GetViewMatrix();
		mat4 projection_mat = mainCamera->GetProjectionMatrix();
		// mat4 mvp = projection_mat * mainCamera->GetViewMatrix() * model_mat; //
		
		Shader::SetMat4(shader_id, "model", model_mat);
		Shader::SetMat4(shader_id, "view", view_mat);
		Shader::SetMat4(shader_id, "proj", projection_mat);
		Shader::SetVec3(shader_id, "cam_pos", mainCamera->GetPosition());

		// 材质属性
		Shader::SetVec3(shader_id, "albedo", render_info.material.albedo);
		Shader::SetFloat(shader_id, "metallic", render_info.material.metallic);
		Shader::SetFloat(shader_id, "roughness", render_info.material.roughness);
		Shader::SetFloat(shader_id, "ao", render_info.material.ao);
		
		bool has_dir_light = false;	// cannot have more than 1 dir light 
		int point_light_count = 0;	// point light count, max 4 
		for(auto light : scene_lights)
		{
			ELightType light_type = light->GetLightType();
			switch (light_type)
			{
			case ELightType::directional_light:
				if(!has_dir_light)
				{
					Shader::SetVec3(shader_id, "directional_light.light_dir", light->rotation);
					Shader::SetVec3(shader_id, "directional_light.light_color", light->light_color);
					has_dir_light = true;
				}
				break;
			case ELightType::point_light:
				if(point_light_count < 4)
				{
					stringstream point_light_name;
					point_light_name <<  "point_lights[" << point_light_count << "]";
					Shader::SetVec3(shader_id, point_light_name.str() + ".light_pos", light->location);
					Shader::SetVec3(shader_id, point_light_name.str() + ".light_color", light->light_color);
					++point_light_count;
				}
				break;
			case ELightType::spot_light:
			default:
				break;
			}
		}
	
		Shader::SetInt(shader_id, "point_light_count", point_light_count);
		// //use shadow map
		// mat4 bias_mat(
		// 	0.5, 0.0, 0.0, 0.0,
		// 	0.0, 0.5, 0.0, 0.0,
		// 	0.0, 0.0, 0.5, 0.0,
		// 	0.5, 0.5, 0.5, 1.0
		// );
		//
		// mat4 depth_bias_mvp = bias_mat * m_DepthMVP;
		//
		// GLuint depth_bias_id = glGetUniformLocation(render_info.program_id, "depth_bias_mvp");
		// glUniformMatrix4fv(depth_bias_id, 1, GL_FALSE, &depth_bias_mvp[0][0]);


		/*
		法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
		Normal = mat3(transpose(inverse(model))) * aNormal;
		 */
		mat3 normal_model_mat = transpose(inverse(model_mat));
		Shader::SetMat3(shader_id, "normal_model_mat", normal_model_mat);
	
		glActiveTexture(GL_TEXTURE0);
		GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
		glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

		glActiveTexture(GL_TEXTURE1);
		GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
		glBindTexture(GL_TEXTURE_2D, specular_map_id);

		glActiveTexture(GL_TEXTURE2);
		GLuint normal_map_id = render_info.normal_tex_id != 0 ? render_info.normal_tex_id : null_tex_id;
		glBindTexture(GL_TEXTURE_2D, normal_map_id);

		glActiveTexture(GL_TEXTURE3);
		GLuint tangent_map_id = render_info.tangent_tex_id != 0 ? render_info.tangent_tex_id : null_tex_id;
		glBindTexture(GL_TEXTURE_2D, tangent_map_id);
	
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
	}
	glBindVertexArray(GL_NONE);	// 解绑VAO
}

GLuint CRender::LoadTexture(const std::string& texture_path)
{
	GLuint texture_id = GL_NONE;
	if (texture_path.empty())
	{
		return texture_id;		
	}
	stbi_set_flip_vertically_on_load(true);
	int width, height, nr_component;
	auto data = stbi_load(texture_path.c_str(), &width, &height, &nr_component, 0);
	assert(data);

	GLenum format = GL_BGR;
	switch(nr_component)
	{
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		break;
	}

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// release memory
	stbi_image_free(data);
	// auto tex_image = new TGAImage;
	// tex_image->read_tga_file(texture_path.c_str());


	return texture_id;
}


void CRender::RenderSkyBox()
{
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrixNoTranslate(); //
	m_SkyBox.Render(mvp);
}

void CRender::RenderShadowMap(const SRenderInfo& render_info)
{		
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
	//
	// // shadow map shader
	// glUseProgram(m_ShadowMapProgramID);	
	//
	// // Compute the MVP matrix from the light's point of view
	// //1024.0f / 768.0f, 0.1f, 500.0f
	// mat4 depth_proj_mat = ortho<float>(-10, 10, -10, 10, -10, 20);
	// mat4 depth_view_mat = lookAt(m_LightDir+m_LightPos, m_LightPos, glm::vec3(0, 1, 0));
	//
	// mat4 depth_model_mat = mat4(1.0);
	// m_DepthMVP = depth_proj_mat * depth_view_mat * depth_model_mat;
	//
	// // in the "MVP" uniform
	// glUniformMatrix4fv(m_DepthMatrixID, 1, GL_FALSE, &m_DepthMVP[0][0]);
	//
	// // 1rst attribute buffer : vertices
	// glEnableVertexAttribArray(0);
	// glBindBuffer(GL_ARRAY_BUFFER, render_info.vertex_buffer);
	// glVertexAttribPointer(
	// 	0,  // The attribute we want to configure
	// 	3,                  // size
	// 	GL_FLOAT,           // type
	// 	GL_FALSE,           // normalized?
	// 	0,                  // stride
	// 	(void*)0            // array buffer offset
	// );	
	//
	// glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle	
	//
	// glDisableVertexAttribArray(0);
}

// void CRender::UpdateLightDir(float delta)
// {
// 	// rotate speed
// 	double light_speed = 30.0;
//
// 	light_yaw += light_speed*delta;
// 	
// 	vec3 front;
// 	front.x = cos(glm::radians(light_yaw)) * cos(glm::radians(light_pitch));
// 	front.y = sin(glm::radians(light_pitch));
// 	front.z = sin(glm::radians(light_yaw)) * cos(glm::radians(light_pitch));
//
// 	m_LightDir = normalize(front);
// 	// printf("--- light dir %f %f %f\n", m_LightDir.x, m_LightDir.y, m_LightDir.z);
// }

