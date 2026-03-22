#include "RenderModule.hpp"
#include "Render/Abstraction/IGraphicsDevice.hpp"
#include "Render/Abstraction/IFrameContext.hpp"
#include "Render/Abstraction/IRenderModuleBackend.hpp"
#include "Render/Abstraction/BackendType.hpp"
#ifndef RENDER_IN_VULKAN
#include "Render/GraphicsAPI/OpenGL/OpenGLRenderPassHost.hpp"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <chrono>
#ifndef RENDER_IN_VULKAN
#include <imgui.h>
#endif
#include <array>
#include <random>

#include "Actor.hpp"
#include "Component/CameraComponent.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "Scene.hpp"
#include "Shader/OpenGL/OpenGLShader.h"
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

static KongRenderModule g_renderModule;

KongRenderModule& KongRenderModule::GetRenderModule()
{
	return g_renderModule;
}

void KongRenderModule::PushRenderSystem(std::unique_ptr<IRenderSystem> sys)
{
	if (sys)
		m_renderSystems.push_back(std::move(sys));
}

KongTexture* KongRenderModule::GetNullTex()
{
	return g_renderModule.m_nullTex.lock().get();
}

vec2 KongRenderModule::GetNearFar()
{
	return g_renderModule.mainCamera->GetNearFar();
}

shared_ptr<CQuadShape> KongRenderModule::GetScreenShape()
{
	return g_renderModule.m_quadShape;
}

KongRenderModule::~KongRenderModule()
{
}

int KongRenderModule::Init(IGraphicsDevice* device)
{
	m_device = device;
	mainCamera = make_shared<CCamera>(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));

	string null_tex_path = RESOURCE_PATH + "Engine/null_texture.png";
	m_nullTex = ResourceManager::GetOrLoadTexture_new(diffuse, null_tex_path);

	if (device)
		m_passHost = device->CreateRenderPassHost();

	if (device && device->GetBackendType() == BackendType::OpenGL)
	{
		m_quadShape = make_shared<CQuadShape>();
		if (m_passHost)
			m_passHost->OnAttached(*this, device);
	}

	if (device)
	{
		m_backend = CreateRenderModuleBackend(device->GetBackendType());
		if (m_backend)
			m_backend->Init(this, device);
	}

	return 0;
}

void KongRenderModule::UpdateSceneRenderInfo()
{
	// todo: 改为和渲染API无关
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
	if(bool has_dir_light = !scene_render_info.scene_dirlight.expired())
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

	if (m_backend)
		m_backend->UpdateSceneRenderInfo(this);
	if (m_passHost)
		m_passHost->OnSceneLightInfoUpdated(*this, light_info);
}


int KongRenderModule::Update(double delta)
{
	return Update(delta, nullptr);
}

int KongRenderModule::Update(double delta, IFrameContext* frameContext)
{
	// 更新相机
	render_time += delta;
	mainCamera->Update(delta);
	UpdateSceneRenderInfo();

	// 统一 RHI 路径：有帧上下文且已注册 Pass 时，按 m_renderSystems 顺序执行
	if (frameContext && !m_renderSystems.empty())
	{
		SceneDrawInfo sceneDrawInfo;
		sceneDrawInfo.frameIndex = frameContext->GetFrameIndex();
		sceneDrawInfo.frameTime = static_cast<float>(delta);
		sceneDrawInfo.sceneContext = this;
		sceneDrawInfo.currentColorRT = static_cast<uintptr_t>(latestRenderResult.resultColor);
		sceneDrawInfo.currentDepthRT = static_cast<uintptr_t>(latestRenderResult.resultDepth);
		sceneDrawInfo.rhiCommandList = frameContext->GetRHICommandList();
		sceneDrawInfo.currentFramebuffer = latestRenderResult.rhiFramebuffer;
		sceneDrawInfo.rhiCurrentColor = latestRenderResult.rhiResultColor;
		sceneDrawInfo.rhiCurrentDepth = latestRenderResult.rhiResultDepth;
		for (auto& sys : m_renderSystems)
			sys->Draw(*frameContext, sceneDrawInfo);
		return 1;
	}

	// 兼容路径：无 frameContext 时由后端执行整帧绘制，无后端时仅做阴影与 UI
	if (m_backend)
		m_backend->DrawFallback(this, delta);
	else
	{
		RenderShadowMap();
		RenderUIBeforeSystems();
	}
	return 1;
}

void KongRenderModule::RenderUIBeforeSystems()
{
	auto main_cam = GetCamera();
#ifndef RENDER_IN_VULKAN
	if (main_cam)
	{
		ImGui::DragFloat("cam exposure", &main_cam->exposure, 0.02f, 0.01f, 10.0f);
		ImGui::DragFloat("cam speed", &main_cam->move_speed, 0.2f, 1.0f, 100.0f);
	}
	ImGui::Checkbox("screen space reflection", &use_screen_space_reflection);
#endif
}

void KongRenderModule::RenderUI(double delta)
{
	(void)delta;
	RenderUIBeforeSystems();
	if (m_passHost && m_passHost->HasSubsystemUI())
		m_passHost->DrawSubsystemUI(*this);
	if (m_backend)
		m_backend->DrawUI(this);
}

RenderResultInfo KongRenderModule::RenderSceneObject(GLuint target_fbo)
{
	(void)target_fbo;
	if (m_device && m_device->GetBackendType() == BackendType::OpenGL && m_passHost)
	{
		m_passHost->DrawMainScene(*this, nullptr);
		return latestRenderResult;
	}
	if (m_backend)
	{
		m_backend->DrawMainScene(this);
		return latestRenderResult;
	}
	return latestRenderResult;
}

void KongRenderModule::RenderNonDeferSceneObjects(int skybox_render_sky_env_status) const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	auto actors = KongSceneManager::GetActors();
	for (auto actor : actors)
	{
		auto mesh_component = actor->GetComponent<CMeshComponent>();
		if (!mesh_component)
			continue;
		auto mesh_shader = mesh_component->shader_data;
		if (dynamic_pointer_cast<DeferInfoShader>(mesh_shader)
			|| dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader)
			|| dynamic_pointer_cast<Water>(mesh_component)
			|| dynamic_pointer_cast<GerstnerWaveWater>(mesh_component))
			continue;

		mesh_shader->Use();
		mesh_shader->SetBool("b_render_skybox", skybox_render_sky_env_status == 1);
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
#ifndef RENDER_IN_VULKAN 
	glCullFace(GL_FRONT);
	glViewport(0,0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
#endif
	
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

#ifndef RENDER_IN_VULKAN
	// 恢复背面剔除，否则后续场景（立方体等）会变成剔除正面
	glCullFace(GL_BACK);
#endif

#if SHADOWMAP_DEBUG
	if (auto* glh = dynamic_cast<OpenGLRenderPassHost*>(m_passHost.get()))
		glh->DrawShadowMapDebug(*this);
#endif
}

void KongRenderModule::OnWindowResize(int width, int height)
{
	if (m_passHost)
		m_passHost->OnWindowResize(*this, width, height);
	if (m_backend)
		m_backend->OnWindowResize(width, height);
}

void KongRenderModule::SetRenderWater(const weak_ptr<AActor>& render_water_actor)
{
	if (m_passHost)
		m_passHost->SetRenderWater(*this, render_water_actor);
	if (m_backend)
		m_backend->SetRenderWater(render_water_actor);
}

void KongRenderModule::OnReloadScene()
{
	if (m_backend)
		m_backend->OnReloadScene(this);
}

#ifndef RENDER_IN_VULKAN
OpenGLRenderSystem* KongRenderModule::GetOpenGLSubsystem(RenderSystemType type)
{
	return m_passHost ? m_passHost->TryGetOpenGLSubsystem(type) : nullptr;
}

IRHIRenderSubsystem* KongRenderModule::GetRHISubsystem(RHISubsystemKind kind)
{
	return m_passHost ? m_passHost->TryGetRHISubsystem(kind) : nullptr;
}

GLuint KongRenderModule::GetMainFBO()
{
	return m_passHost ? static_cast<GLuint>(m_passHost->GetMainFBONativeHandle()) : 0;
}

GLuint KongRenderModule::GetMainColorTexture(unsigned index)
{
	return m_passHost ? static_cast<GLuint>(m_passHost->GetMainColorTextureNativeHandle(index)) : 0;
}

IFramebuffer* KongRenderModule::GetMainSceneFramebuffer()
{
	return m_passHost ? m_passHost->GetMainSceneFramebuffer(*this) : nullptr;
}
#else
OpenGLRenderSystem* KongRenderModule::GetOpenGLSubsystem(RenderSystemType)
{
	return nullptr;
}

IRHIRenderSubsystem* KongRenderModule::GetRHISubsystem(RHISubsystemKind kind)
{
	return m_passHost ? m_passHost->TryGetRHISubsystem(kind) : nullptr;
}

GLuint KongRenderModule::GetMainFBO()
{
	return 0;
}

GLuint KongRenderModule::GetMainColorTexture(unsigned)
{
	return 0;
}

IFramebuffer* KongRenderModule::GetMainSceneFramebuffer()
{
	return nullptr;
}
#endif