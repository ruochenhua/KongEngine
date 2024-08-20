#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Actor.h"
#include "Component/CameraComponent.h"
#include "Engine.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "message.h"
#include "Scene.h"
#include "Shader/Shader.h"
#include "stb_image.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

map<string, GLuint> texture_manager;

CRender* g_render = new CRender;

void UBOHelper::Init(GLuint in_binding)
{
	binding = in_binding;
	// 创建UBO
	glGenBuffers(1, &ubo_idx);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_idx);
	glBufferData(GL_UNIFORM_BUFFER, next_offset, NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_idx);
	glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
}

void UBOHelper::Bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_idx);
}

void UBOHelper::EndBind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

CRender* CRender::GetRender()
{
	return g_render;
}
GLuint CRender::GetNullTexId()
{
	return g_render->null_tex_id;
}

GLuint CRender::GetSkyboxTexture() const
{
	return m_SkyBox.GetSkyBoxTextureId();
}

GLuint CRender::GetSkyboxDiffuseIrradianceTexture() const
{
	return m_SkyBox.GetDiffuseIrradianceTexture();
}

GLuint CRender::GetSkyboxPrefilterTexture() const
{
	return m_SkyBox.GetPrefilterTexture();
}

GLuint CRender::GetSkyboxBRDFLutTexture() const
{
	return m_SkyBox.GetBRDFLutTexture();
}

int CRender::Init()
{
	render_window = Engine::GetRenderWindow();
	InitCamera();
	
	m_SkyBox.Init();
#if SHADOWMAP_DEBUG
	map<EShaderType, string> debug_shader_paths = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.frag")}
	};
	shadowmap_debug_shader = make_shared<Shader>();

	shadowmap_debug_shader->Init(debug_shader_paths);
	shadowmap_debug_shader->Use();
	shadowmap_debug_shader->SetInt("shadow_map", 0);
#endif
	
	// load null texture
	string null_tex_path = RESOURCE_PATH + "Engine/null_texture.png";
	null_tex_id = LoadTexture(null_tex_path);

	InitUBO();
	post_process.Init();
	
	return 0;
}

int CRender::InitCamera()
{
	mainCamera = new CCamera(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	mainCamera->InitControl();
	return 0;
}

void CRender::InitUBO()
{
	// 初始化UBO数据
	matrix_ubo.AppendData(glm::mat4(), "model");
	matrix_ubo.AppendData(glm::mat4(), "view");
	matrix_ubo.AppendData(glm::mat4(), "projection");
	matrix_ubo.AppendData(glm::vec3(), "cam_pos");
	matrix_ubo.Init(0);

	scene_light_ubo.AppendData(SceneLightInfo(), "light_info");
	scene_light_ubo.Init(1);
}

int CRender::Update(double delta)
{
	mainCamera->Update(delta);		
	// RenderSkyBox();
	CollectLightInfo();
	
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if !SHADOWMAP_DEBUG
	int width = Engine::GetEngine().GetWindowWidth();
	int height = Engine::GetEngine().GetWindowHeight();

	glViewport(0,0, width, height);
	// 渲染到后处理framebuffer上
	glBindFramebuffer(GL_FRAMEBUFFER, post_process.GetScreenFrameBuffer());
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(b_render_skybox)
	{
		RenderSkyBox();	
	}
	
	RenderScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
	post_process.Draw();
#endif
}

void CRender::ChangeSkybox()
{
	m_SkyBox.ChangeSkybox();
}

GLuint CRender::LoadTexture(const std::string& texture_path, bool flip_uv)
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
	stbi_set_flip_vertically_on_load(flip_uv);
	int width, height, nr_component;
	auto data = stbi_load(texture_path.c_str(), &width, &height, &nr_component, 0);
	assert(data, "load texture failed");

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
	//mat4 mvp = projection * mainCamera->GetViewMatrix(); //
	m_SkyBox.Render(mvp);
}

void CRender::RenderScene() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	// 更新光源UBO
	SceneLightInfo light_info;
	bool has_dir_light = !scene_render_info.scene_dirlight.expired(); 
	if(has_dir_light)
	{
		light_info.has_dir_light = ivec4(1);
		auto dir_light = scene_render_info.scene_dirlight.lock();
		light_info.directional_light.light_dir = vec4(dir_light->GetLightDir(), 1.0);
		light_info.directional_light.light_color = vec4(dir_light->light_color, 1.0);
		light_info.directional_light.light_space_mat = dir_light->light_space_mat;
	}
	else
	{
		light_info.has_dir_light = ivec4(0);
	}
			
	int point_light_count = 0;
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(point_light_count > 3)
		{
			break;
		}
				
		if(light.expired())
		{
			continue;
		}
		PointLight point_light;
		point_light.light_pos = vec4(light.lock()->GetLightLocation(), 1.0);
		point_light.light_color = vec4(light.lock()->light_color, 1.0);

		light_info.point_lights[point_light_count] = point_light; 				
		++point_light_count;
	}
			
	light_info.point_light_count = ivec4(point_light_count);
	
	scene_light_ubo.Bind();
	scene_light_ubo.UpdateData(light_info, "light_info");
	scene_light_ubo.EndBind();
	
	// 更新UBO里的矩阵数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(scene_render_info.camera_view, "view");
	matrix_ubo.UpdateData(scene_render_info.camera_proj, "projection");
	matrix_ubo.UpdateData(scene_render_info.camera_pos, "cam_pos");
	matrix_ubo.EndBind();
	
	auto actors = CScene::GetActors();
	for(auto actor : actors)
	{
		auto mesh_component = actor->GetComponent<CMeshComponent>();
		if(!mesh_component)
		{
			continue;
		}

		matrix_ubo.Bind();
		matrix_ubo.UpdateData(actor->GetModelMatrix(), "model");
		matrix_ubo.EndBind();

		mesh_component->shader_data->Use();
		mesh_component->shader_data->SetBool("b_render_skybox", b_render_skybox);
		mesh_component->Draw(scene_render_info);
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
		if(!light_component)
		{
			continue;
		}

		auto dir_light = std::dynamic_pointer_cast<CDirectionalLightComponent>(light_component);
		if(dir_light)
		{
			dir_light->SetLightDir(actor->rotation);
			scene_render_info.scene_dirlight = dir_light;
			continue;
		}

		if(scene_render_info.scene_pointlights.size() < 4)
		{
			auto point_light = dynamic_pointer_cast<CPointLightComponent>(light_component);
			if(point_light)
			{
				point_light->SetLightLocation(actor->location);
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

