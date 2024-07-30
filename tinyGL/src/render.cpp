#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Camera.h"
#include "Engine.h"
#include "light.h"
#include "model.h"
#include "tgaimage.h"
#include "message.h"
#include "Scene.h"
#include "shader.h"
#include "stb_image.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

CRender* g_render = new CRender;
CRender* CRender::GetRender()
{
	return g_render;
}

int CRender::Init()
{
	render_window = Engine::GetRenderWindow();
	InitCamera();

	// m_SkyBox.Init();
	// init shadow map
	glGenFramebuffers(1, &m_ShadowMapFBO);
	
	glGenTextures(1, &m_DepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMapFBO, 0);
	
	// 我们需要的只是在从光的透视图下渲染场景的时候深度信息，所以颜色缓冲没有用。
	// 然而，不包含颜色缓冲的帧缓冲对象是不完整的，所以我们需要显式告诉OpenGL我们不适用任何颜色数据进行渲染。
	// 我们通过将调用glDrawBuffer和glReadBuffer把读和绘制缓冲设置为GL_NONE来做这件事。
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	// 	return -1;
	//
	map<SRenderResourceDesc::EShaderType, string> shader_paths = {
		{SRenderResourceDesc::EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap.vert")},
		{SRenderResourceDesc::EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap.frag")}
	};
	m_ShadowMapProgramID = Shader::LoadShaders(shader_paths);

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

void CRender::RenderSceneObject()
{
	float width = Engine::GetEngine().GetWindowWidth();
	float height = Engine::GetEngine().GetWindowHeight();
	glViewport(0,0,width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	RenderScene();
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

void CRender::RenderScene() const
{
	auto render_objs = CScene::GetScene()->GetSceneRenderObjects();
	for(auto render_obj : render_objs)
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
			auto scene_lights = CScene::GetScene()->GetSceneLights();
			for(auto light : scene_lights)
			{
				ELightType light_type = light->GetLightType();
				switch (light_type)
				{
				case ELightType::directional_light:
					if(!has_dir_light)
					{
						Shader::SetVec3(shader_id, "directional_light.light_dir", light->GetLightDir());
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
			Shader::SetMat4(shader_id, "light_space_mat", light_space_mat);
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

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
		
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
}

void CRender::RenderShadowMap()
{
	glViewport(0,0,1024,1024);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	// shadow and matrices
	GLfloat near_plane = 1.f;
	GLfloat far_plane = 20.f;
	// 光线投影采用正交投影矩阵
	mat4 light_proj = ortho(-10.f, 10.f, -10.f, 10.f, near_plane, far_plane);
	vec3 light_dir = vec3(1, -1, 0);	// 默认写一个
	auto scene_lights = CScene::GetScene()->GetSceneLights();
	for(auto slight : scene_lights)
	{
		if(slight->GetLightType() == ELightType::directional_light)
		{
			light_dir = slight->GetLightDir();
			break;
		}
	}

	vec3 light_pos = light_dir * -10.f;
	mat4 light_view = lookAt(light_pos, vec3(0,0,0), vec3(0, 1, 0));
	light_space_mat = light_proj * light_view;

	glUseProgram(m_ShadowMapProgramID);
	Shader::SetMat4(m_ShadowMapProgramID, "light_space_mat", light_space_mat);
	// RenderScene();

	auto render_objs = CScene::GetScene()->GetSceneRenderObjects();
	for(auto render_obj : render_objs)
	{
		for(auto& mesh : render_obj->mesh_list)
		{
			const SRenderInfo& render_info = mesh.GetRenderInfo();
			glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
		
			mat4 model_mat = render_obj->GetModelMatrix();
			// mat4 mvp = projection_mat * mainCamera->GetViewMatrix() * model_mat; //
			
			Shader::SetMat4(m_ShadowMapProgramID, "model", model_mat);
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
	
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
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

