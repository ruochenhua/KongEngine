#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Actor.h"
#include "CameraComponent.h"
#include "Engine.h"
#include "LightComponent.h"
#include "MeshComponent.h"
#include "model.h"
#include "tgaimage.h"
#include "message.h"
#include "Scene.h"
#include "shader.h"
#include "stb_image.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

map<string, GLuint> texture_manager;

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
	map<SRenderResourceDesc::EShaderType, string> debug_shader_paths = {
		{SRenderResourceDesc::EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.vert")},
		{SRenderResourceDesc::EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.frag")}
	};
	m_ShadowMapDebugShaderId = Shader::LoadShaders(debug_shader_paths);
	glUseProgram(m_ShadowMapDebugShaderId);
	Shader::SetInt(m_ShadowMapDebugShaderId, "shadow_map", 0);
	
	// load null texture
	string null_tex_path = RESOURCE_PATH + "Engine/null_texture.png";
	null_tex_id = LoadTexture(null_tex_path);
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
	// glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
	
	//RenderSkyBox();
	CollectLightInfo();
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	RenderShadowMap();			
	RenderSceneObject();	
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
	glCullFace(GL_BACK);
#if SHADOWMAP_DEBUG
#else
	int width = Engine::GetEngine().GetWindowWidth();
	int height = Engine::GetEngine().GetWindowHeight();
	glViewport(0,0,width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	RenderScene();
#endif
}

GLuint CRender::LoadTexture(const std::string& texture_path)
{
	auto texture_iter = texture_manager.find(texture_path);
	if(texture_iter != texture_manager.end())
	{
		return texture_iter->second;
	}
	
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
	// tex_image->read_tga_file(texture_path.c_str());
	texture_manager.emplace(texture_path, texture_id);
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
	auto actors = CScene::GetActors();

	for(auto actor : actors)
	{
		auto mesh_component = actor->GetComponent<CMeshComponent>();
		if(mesh_component.expired())
		{
			continue;
		}

		auto render_obj = mesh_component.lock();
		GLuint shader_id = render_obj->GetShaderId();

		CTransformComponent* transform_component_ptr = nullptr;
		auto tranform_component = actor->GetComponent<CTransformComponent>();
		if(!tranform_component.expired())
		{
			transform_component_ptr = tranform_component.lock().get();
		}
		if(!transform_component_ptr)
		{
			continue;
		}

		glUseProgram(shader_id);
		unsigned mesh_idx = 0;
		for(auto& mesh : render_obj->mesh_list)
		{
			const SRenderInfo& render_info = mesh.GetRenderInfo();
			glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
			
			mat4 model_mat;

			if(transform_component_ptr->instancing_info.count == 0)
			{
				model_mat = actor->GetModelMatrix();
			}
			else
			{
				model_mat = transform_component_ptr->GetInstancingModelMat(mesh_idx);
			}
			
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
			
			int point_light_count = 0;	// point light count, max 4

			if(!scene_dirlight.expired())
			{
				Shader::SetVec3(shader_id, "directional_light.light_dir", scene_dirlight.lock()->GetLightDir());
				Shader::SetVec3(shader_id, "directional_light.light_color", scene_dirlight.lock()->light_color);
					
				Shader::SetMat4(shader_id, "light_space_mat", scene_dirlight.lock()->light_space_mat);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, scene_dirlight.lock()->shadowmap_texture);
			}
			
			for(auto light : scene_pointlights)
			{
				if(light.expired())
				{
					continue;
				}
				
				stringstream point_light_name;
				point_light_name <<  "point_lights[" << point_light_count << "]";
				Shader::SetVec3(shader_id, point_light_name.str() + ".light_pos", light.lock()->GetLightLocation());
				Shader::SetVec3(shader_id, point_light_name.str() + ".light_color", light.lock()->light_color);
				// 先支持一个点光源的阴影贴图
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_CUBE_MAP, light.lock()->shadowmap_texture);
			
				++point_light_count;
			}
	
			Shader::SetInt(shader_id, "point_light_count", point_light_count);
		
			// Draw the triangle !
			// if no index, use draw array
			
			if(render_info.index_buffer == GL_NONE)
			{
				if(transform_component_ptr->instancing_info.count > 0)
				{
					// Starting from vertex 0; 3 vertices total -> 1 triangle
					glDrawArraysInstanced(GL_TRIANGLES, 0,
						render_info.vertex_size / render_info.stride_count,
						transform_component_ptr->instancing_info.count);
				}
				else
				{
					// Starting from vertex 0; 3 vertices total -> 1 triangle
					glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); 	
				}
			}
			else
			{		
				glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
			}
			mesh_idx++;
		
		}
		glBindVertexArray(GL_NONE);	// 解绑VAO
	}
}

void CRender::CollectLightInfo()
{
	// todo: scene重新加载的时候处理一次就好
	scene_dirlight.reset();
	scene_pointlights.clear();
	
	auto actors = CScene::GetActors();
	for(auto actor: actors)
	{
		auto light_component = actor->GetComponent<CLightComponent>();
		if(light_component.expired())
		{
			continue;
		}

		// light需要tranform信息，没有就跳过
		auto transform_component = actor->GetComponent<CTransformComponent>();
		if(transform_component.expired())
		{
			continue;
		}
		
		auto light_sharedptr = light_component.lock();
		if(scene_dirlight.expired())
		{
			auto dir_light = std::dynamic_pointer_cast<CDirectionalLightComponent>(light_sharedptr);
			if(dir_light)
			{
				dir_light->SetLightDir(normalize(transform_component.lock()->rotation));
				scene_dirlight = dir_light;
				
				continue;
			}
		}

		if(scene_pointlights.size() < 4)
		{
			auto point_light = dynamic_pointer_cast<CPointLightComponent>(light_sharedptr);
			if(point_light)
			{
				point_light->SetLightLocation(transform_component.lock()->location);
				scene_pointlights.push_back(point_light);
			}
		}
	}
}

void CRender::RenderShadowMap()
{
	// shadowmap 需要正面剔除，避免阴影悬浮
	// todo: 处理内部有开口模型或者平面该如何处理？
	// note: 剔除front好像shadow bias不填阴影效果也比较正常？
	glCullFace(GL_FRONT);
	glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);

	// auto scene_lights = CScene::GetScene()->GetSceneLights();
	if(!scene_dirlight.expired())
	{
		scene_dirlight.lock()->RenderShadowMap();
	}
	
	for(auto light : scene_pointlights)
	{
		if(!light.expired())
		{
			light.lock()->RenderShadowMap();
		}
	}

#if SHADOWMAP_DEBUG
	glCullFace(GL_BACK);
	// 先只画平行光的
	DirectionalLight* dir_light = nullptr;
	for(auto light : scene_lights)
	{
		if(light->GetLightType() == ELightType::directional_light)
		{
			dir_light = static_cast<DirectionalLight*>(light.get());
			break;
		}
	}

	if(!dir_light)
		return;
	
	int width = Engine::GetEngine().GetWindowWidth();
	int height = Engine::GetEngine().GetWindowHeight();
	glViewport(0,0,width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_ShadowMapDebugShaderId);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "near_plane", dir_light->near_plane);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "far_plane", dir_light->far_plane);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, dir_light->m_DepthTexture);

	// renderQuad() renders a 1x1 XY quad in NDC
	// -----------------------------------------
	if (m_QuadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &m_QuadVAO);
		glGenBuffers(1, &m_QuadVBO);
		glBindVertexArray(m_QuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(m_QuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
#endif
	
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

