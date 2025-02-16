#include "RenderModule.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <chrono>
#include <imgui.h>
#include <random>

#include "Actor.hpp"
#include "Component/CameraComponent.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "Scene.hpp"
#include "Shader/Shader.h"
#include "stb_image.h"
#include "Render/Resource/Texture.hpp"
#include "Window.hpp"
#include "Component/Mesh/GerstnerWaveWater.h"
#include "Component/Mesh/QuadShape.h"
#include "Component/Mesh/Water.h"
#include "glm/gtx/dual_quaternion.hpp"

using namespace Kong;
using namespace glm;
using namespace std;
#define DEFER_TERRAIN true

static KongRenderModule g_renderModule;

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
#if !USE_DSA
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_idx);
#endif
}

void UBOHelper::EndBind() const
{
#if !USE_DSA
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif
}

KongRenderModule& KongRenderModule::GetRenderModule()
{
	return g_renderModule;
}
GLuint KongRenderModule::GetNullTexId()
{
	return g_renderModule.null_tex_id;
}

vec2 KongRenderModule::GetNearFar()
{
	return g_renderModule.mainCamera->GetNearFar();
}

shared_ptr<CQuadShape> KongRenderModule::GetScreenShape()
{
	return g_renderModule.m_quadShape;
}

int KongRenderModule::Init()
{
	InitMainFBO();
	
	mainCamera = make_shared<CCamera>(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));
	m_quadShape = make_shared<CQuadShape>();

#if SHADOWMAP_DEBUG
	map<EShaderType, string> debug_shader_paths = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_debug.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_debug.frag")}
	};
	shadowmap_debug_shader = make_shared<Shader>();

	shadowmap_debug_shader->Init(debug_shader_paths);
	shadowmap_debug_shader->Use();
	shadowmap_debug_shader->SetInt("shadow_map", 0);
#endif
	
	// load null texture
	string null_tex_path = RESOURCE_PATH + "Engine/null_texture.png";
	null_tex_id = ResourceManager::GetOrLoadTexture(null_tex_path);

	InitUBO();
	
	// add render system
	m_skyboxRenderSystem.Init();
	m_deferRenderSystem.Init();
	m_postProcessRenderSystem.Init();
	m_ssReflectionRenderSystem.Init();
	m_waterRenderSystem.Init();

	return 0;
}

KongRenderSystem* KongRenderModule::GetRenderSystemByType(RenderSystemType type)
{
	switch (type)
	{
	case RenderSystemType::SKYBOX:
		return &m_skyboxRenderSystem;
		
	case RenderSystemType::DEFERRED:
		return &m_deferRenderSystem;

	case RenderSystemType::POST_PROCESS:
		return &m_postProcessRenderSystem;

	case RenderSystemType::SS_REFLECTION:
		return &m_ssReflectionRenderSystem;
	
	default:
		throw std::exception("Render system type not found");
	}
}

int KongRenderModule::InitCamera()
{
	
	
	return 0;
}

void KongRenderModule::UpdateSceneRenderInfo()
{
	scene_render_info.clear();
	auto actors = KongSceneManager::GetActors();
	for(auto actor: actors)
	{
		auto light_component = actor->GetComponent<CLightComponent>();
		if(!light_component)
		{
			continue;
		}

		auto dir_light =
			std::dynamic_pointer_cast<CDirectionalLightComponent>(light_component);
		
		if(dir_light)
		{
			dir_light->SetLightDir(actor->rotation);
			scene_render_info.scene_dirlight = dir_light;
			continue;
		}

		if(scene_render_info.scene_pointlights.size() < POINT_LIGHT_MAX)
		{
			auto point_light = dynamic_pointer_cast<CPointLightComponent>(light_component);
			if(point_light)
			{
				point_light->SetLightLocation(actor->location);
				scene_render_info.scene_pointlights.push_back(point_light);
			}
		}
	}
	
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
	int point_light_shadow_count = 0;
	light_info.point_light_shadow_index = ivec4(-1);
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(point_light_count >= POINT_LIGHT_MAX)
		{
			break;
		}
				
		if(light.expired())
		{
			continue;
		}
		auto point_light_ptr = light.lock();
		PointLight point_light;
		point_light.light_pos = vec4(point_light_ptr->GetLightLocation(), 1.0);
		point_light.light_color = vec4(point_light_ptr->light_color, 1.0);

		light_info.point_lights[point_light_count] = point_light;
		if(point_light_ptr->enable_shadowmap && point_light_shadow_count<POINT_LIGHT_SHADOW_MAX)
		{
			light_info.point_light_shadow_index[point_light_shadow_count] = point_light_count;
			++point_light_shadow_count;
		}
		
		++point_light_count;
	}
			
	light_info.point_light_count = ivec4(point_light_count);
	
	scene_light_ubo.Bind();
	scene_light_ubo.UpdateData(light_info, "light_info");
	scene_light_ubo.EndBind();
}


void KongRenderModule::InitUBO()
{
	// 初始化UBO数据
//	matrix_ubo.AppendData(glm::mat4(), "model");
	matrix_ubo.AppendData(glm::mat4(), "view");
	matrix_ubo.AppendData(glm::mat4(), "projection");
	matrix_ubo.AppendData(glm::vec4(), "cam_pos");
	matrix_ubo.AppendData(glm::vec4(), "near_far");
	matrix_ubo.Init(0);

	scene_light_ubo.AppendData(SceneLightInfo(), "light_info");
	scene_light_ubo.Init(1);

	// 更新远近平面数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(vec4(mainCamera->GetNearFar(), 0, 0), "near_far");
}

void KongRenderModule::InitMainFBO()
{
	auto window_size = KongWindow::GetWindowModule().windowSize;
	int width = window_size.x;
	int height = window_size.y;
	
	glGenFramebuffers(1, &m_renderToBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_renderToBuffer);
	
	TextureCreateInfo fragout_texture_create_info
	{
	    GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT,
	    width, height, GL_REPEAT, GL_REPEAT, GL_REPEAT,
	    GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
	};
	
	
	for(unsigned i = 0; i < FRAGOUT_TEXTURE_COUNT; ++i)
	{
	    TextureBuilder::CreateTexture(m_renderToTextures[i], fragout_texture_create_info);
	    
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i,
	        GL_TEXTURE_2D, m_renderToTextures[i], 0);
	}
	// depth buffer
	if(!m_renderToRbo)
	{
	    // 注意这里不是glGenTextures，搞错了查了半天
	    glGenRenderbuffers(1, &m_renderToRbo);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderToRbo);
	
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderToRbo);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	// 渲染到多个颜色附件上
	GLuint color_attachment[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};  
	glDrawBuffers(3, color_attachment); 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


int KongRenderModule::Update(double delta)
{
	mainCamera->Update(delta);		
	render_time += delta;
	// 更新场景信息
	UpdateSceneRenderInfo();
	m_skyboxRenderSystem.PreRenderUpdate();
	
	RenderShadowMap();
	// 更新UBO里的相机数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(mainCamera->GetViewMatrix(), "view");
	matrix_ubo.UpdateData(mainCamera->GetProjectionMatrix(), "projection");
	matrix_ubo.UpdateData(mainCamera->GetPosition(), "cam_pos");
	matrix_ubo.EndBind();
	
	// 普通渲染场景
	latestRenderResult = RenderSceneObject();
	latestRenderResult = m_waterRenderSystem.Draw(delta, latestRenderResult, this);

	// do post process
	latestRenderResult = m_postProcessRenderSystem.Draw(0.0, latestRenderResult, this);

	RenderUI(delta);
	return 1;
}

void KongRenderModule::RenderUI(double delta)
{	
	auto main_cam = GetCamera();
	if(main_cam)
	{
		ImGui::DragFloat("cam exposure", &main_cam->exposure, 0.02f,0.01f, 10.0f);
		ImGui::DragFloat("cam speed", &main_cam->move_speed, 0.2f,1.0f, 100.0f);
	}
	
	ImGui::Checkbox("screen space reflection", &use_screen_space_reflection);

	m_skyboxRenderSystem.DrawUI();
	m_deferRenderSystem.DrawUI();
	
	m_postProcessRenderSystem.DrawUI();
}

RenderResultInfo KongRenderModule::RenderSceneObject(GLuint target_fbo)
{
	RenderResultInfo render_result_info {latestRenderResult};
#if !SHADOWMAP_DEBUG
	// 延迟渲染需要先关掉混合，否则混合操作可能会导致延迟渲染的各个参数贴图的a/w通道影响rgb/xyz值的情况
	glDisable(GL_BLEND);
	ivec2 window_size = KongWindow::GetWindowModule().windowSize;

	// render_scene_texture是场景渲染到屏幕上的未经过后处理的结果
	GLuint render_scene_buffer = target_fbo == GL_NONE ? m_renderToBuffer : target_fbo;
	// 正常渲染到后处理的buffer上
	render_result_info.frameBuffer = render_scene_buffer;
	render_result_info.resultColor = m_renderToTextures[0];
	glViewport(0,0, window_size.x, window_size.y);
	
	render_result_info = m_deferRenderSystem.Draw(0.0, render_result_info, this);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	RenderNonDeferSceneObjects();
	render_result_info = m_skyboxRenderSystem.Draw(0.0, render_result_info, this);
		
	// screen space reflection先放在这里吧
	// 水面反射不做这个
	if(use_screen_space_reflection)
	{
		// 屏幕空间反射的信息渲染到后处理buffer的第三个color attachment贴图中，后通过后处理合成
		render_result_info = m_ssReflectionRenderSystem.Draw(0.0, render_result_info, this);
	}
#endif
	return render_result_info;
}

void KongRenderModule::RenderNonDeferSceneObjects() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	
	auto actors = KongSceneManager::GetActors();
	for(auto actor : actors)
	{
		auto mesh_component = actor->GetComponent<CMeshComponent>();
		if(!mesh_component)
		{
			continue;
		}
		auto mesh_shader = mesh_component->shader_data;
		// 跳过延迟渲染的mesh和水体的部分
		if(dynamic_pointer_cast<DeferInfoShader>(mesh_shader)
#if DEFER_TERRAIN
			|| dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader)
#endif
			|| dynamic_pointer_cast<Water>(mesh_component)
			|| dynamic_pointer_cast<GerstnerWaveWater>(mesh_component))
		{
			continue;
		}

		// 等于1代表渲染skybox，会需要用到环境贴图
		mesh_shader->Use();
		mesh_shader->SetBool("b_render_skybox", m_skyboxRenderSystem.render_sky_env_status == 1);
		mesh_shader->SetMat4("model", actor->GetModelMatrix());
		mesh_shader->SetDouble("iTime", render_time);
		mesh_component->Draw();
	}
}

void KongRenderModule::RenderShadowMap()
{
	// shadowmap 需要正面剔除，避免阴影悬浮
	// todo: 处理内部有开口模型或者平面该如何处理？
	// note: 剔除front好像shadow bias不填阴影效果也比较正常？
	glCullFace(GL_FRONT);
	glViewport(0,0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);

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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	auto window_size = Engine::GetEngine().GetWindowSize();
	int width = window_size.x, height = window_size.y;

	glViewport(0,0,width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 先只画平行光的
	CDirectionalLightComponent* dir_light = scene_render_info.scene_dirlight.lock().get();
	
	if(!dir_light)
		return;
	shadowmap_debug_shader->Use();
	// glUseProgram(m_ShadowMapDebugShaderId);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "near_plane", dir_light->near_plane);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "far_plane", dir_light->far_plane);
	glActiveTexture(GL_TEXTURE0);

	GLuint dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
#if USE_CSM
	glBindTexture(GL_TEXTURE_2D_ARRAY, dir_light_shadowmap_id);
#else
	glBindTexture(GL_TEXTURE_2D, dir_light_shadowmap_id);
#endif
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

void KongRenderModule::OnWindowResize(int width, int height)
{
	m_postProcessRenderSystem.OnWindowResize(width, height);
	//defer_buffer_.GenerateDeferRenderTextures(width, height);
	//ssao_helper_.GenerateSSAOTextures(width, height);
	// water_render_helper_.GenerateWaterRenderTextures(width, height);
}

void KongRenderModule::SetRenderWater(const weak_ptr<AActor>& render_water_actor)
{
	m_waterRenderSystem.m_waterActor = render_water_actor;
}
