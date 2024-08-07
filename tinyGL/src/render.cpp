#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Actor.h"
#include "CameraComponent.h"
#include "Engine.h"
#include "LightComponent.h"
#include "MeshComponent.h"
#include "message.h"
#include "Scene.h"
#include "Shader/Shader.h"
#include "stb_image.h"
#include "Shader/BRDFShader.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

map<string, GLuint> texture_manager;

CRender* g_render = new CRender;
CRender* CRender::GetRender()
{
	return g_render;
}
GLuint CRender::GetNullTexId()
{
	return g_render->null_tex_id;
}
int CRender::Init()
{
	render_window = Engine::GetRenderWindow();
	InitCamera();
	
	// m_SkyBox.Init();
	map<EShaderType, string> debug_shader_paths = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.frag")}
	};
	shadowmap_debug_shader = make_shared<Shader>();
	// m_ShadowMapDebugShaderId = Shader::LoadShaders(debug_shader_paths);
	// glUseProgram(m_ShadowMapDebugShaderId);
	// Shader::SetInt(m_ShadowMapDebugShaderId, "shadow_map", 0);
	shadowmap_debug_shader->Init(debug_shader_paths);
	shadowmap_debug_shader->Use();
	shadowmap_debug_shader->SetInt("shadow_map", 0);
	
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
	// stbi_set_flip_vertically_on_load(true);
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
		
		auto& shader_data  = render_obj->shader_data;
		shader_data->Use();
		unsigned mesh_idx = 0;
		for(auto& mesh : render_obj->mesh_list)
		{
			mat4 model_mat;
			if(transform_component_ptr->instancing_info.count == 0)
			{
				model_mat = actor->GetModelMatrix();
			}
			else
			{
				model_mat = transform_component_ptr->GetInstancingModelMat(mesh_idx);
			}

			shader_data->UpdateRenderData(mesh, model_mat, scene_render_info);
			// Draw the triangle !
			// if no index, use draw array
			auto& render_info = mesh.m_RenderInfo;
			if(render_info.index_buffer == GL_NONE)
			{
				if(render_info.instance_buffer != GL_NONE)
				{
					// Starting from vertex 0; 3 vertices total -> 1 triangle
					glDrawArraysInstanced(GL_TRIANGLES, 0,
						render_info.vertex_size / render_info.stride_count,
						render_info.instance_count);
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
	// todo: scene重新加载的时候处理一次就好,但是transform更新需要及时
	scene_render_info.clear();
	// scene_dirlight.reset();
	// scene_pointlights.clear();
	//
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

		auto dir_light = std::dynamic_pointer_cast<CDirectionalLightComponent>(light_sharedptr);
		if(dir_light)
		{
			dir_light->SetLightDir(transform_component.lock()->rotation);
			scene_render_info.scene_dirlight = dir_light;
			continue;
		}
		

		if(scene_render_info.scene_pointlights.size() < 4)
		{
			auto point_light = dynamic_pointer_cast<CPointLightComponent>(light_sharedptr);
			if(point_light)
			{
				point_light->SetLightLocation(transform_component.lock()->location);
				scene_render_info.scene_pointlights.push_back(point_light);
			}
		}
	}

	scene_render_info.camera_pos = mainCamera->GetPosition();
	scene_render_info.camera_view = mainCamera->GetViewMatrix();
	scene_render_info.camera_proj = mainCamera->GetProjectionMatrix();
		
}

void CRender::RenderShadowMap()
{
	// shadowmap 需要正面剔除，避免阴影悬浮
	// todo: 处理内部有开口模型或者平面该如何处理？
	// note: 剔除front好像shadow bias不填阴影效果也比较正常？
	glCullFace(GL_FRONT);
	glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);

	// auto scene_lights = CScene::GetScene()->GetSceneLights();
	if(!scene_render_info.scene_dirlight.expired())
	{
		scene_render_info.scene_dirlight.lock()->RenderShadowMap();
	}
	
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(!light.expired())
		{
			light.lock()->RenderShadowMap();
		}
	}

#if SHADOWMAP_DEBUG
	glCullFace(GL_BACK);
	// 先只画平行光的
	CDirectionalLightComponent* dir_light = scene_render_info.scene_dirlight.lock().get();
	
	if(!dir_light)
		return;
	
	int width = Engine::GetEngine().GetWindowWidth();
	int height = Engine::GetEngine().GetWindowHeight();
	glViewport(0,0,width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shadowmap_debug_shader->Use();
	// glUseProgram(m_ShadowMapDebugShaderId);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "near_plane", dir_light->near_plane);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "far_plane", dir_light->far_plane);
	glActiveTexture(GL_TEXTURE0);

	GLuint dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
	glBindTexture(GL_TEXTURE_2D, dir_light_shadowmap_id);

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

