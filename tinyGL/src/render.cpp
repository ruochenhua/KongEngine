#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include <random>

#include "Actor.h"
#include "Component/CameraComponent.h"
#include "Engine.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "Scene.h"
#include "Shader/Shader.h"
#include "stb_image.h"
#include "Component/Mesh/QuadShape.h"
#include "glm/gtx/dual_quaternion.hpp"

using namespace Kong;
using namespace glm;
using namespace std;


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

void DeferBuffer::Init(unsigned width, unsigned height)
{
	glGenFramebuffers(1, &g_buffer_);
	GenerateDeferRenderTextures(width, height);

	defer_render_shader = make_shared<DeferredBRDFShader>();
	defer_render_shader->InitDefaultShader();
}

void DeferBuffer::GenerateDeferRenderTextures(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);

	if(g_position_)
	{
		glDeleteTextures(1, &g_position_);
	}
	// 将当前视野的数据用贴图缓存
	// 位置数据
	glGenTextures(1, &g_position_);
	glBindTexture(GL_TEXTURE_2D, g_position_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position_, 0);

	if(g_normal_)
	{
		glDeleteTextures(1, &g_normal_);
	}
	// 法线数据
	glGenTextures(1, &g_normal_);
	glBindTexture(GL_TEXTURE_2D, g_normal_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal_, 0);

	if(g_albedo_)
	{
		glDeleteTextures(1, &g_albedo_);
	}
	// 顶点颜色数据
	glGenTextures(1, &g_albedo_);
	glBindTexture(GL_TEXTURE_2D, g_albedo_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_albedo_, 0);

	// orm数据（ao，roughness，metallic）
	if(g_orm_)
	{
		glDeleteTextures(1, &g_orm_);
	}
	glGenTextures(1, &g_orm_);
	glBindTexture(GL_TEXTURE_2D, g_orm_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_orm_, 0);

	if(g_rbo_)
	{
		glDeleteRenderbuffers(1, &g_rbo_);
	}
	// 生成renderbuffer
	glGenRenderbuffers(1, &g_rbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, g_rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_rbo_);
	glEnable(GL_DEPTH_TEST);
	
	unsigned int attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(4, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOHelper::Init(int width, int height)
{
	// ssao初始化
	glGenFramebuffers(1, &ssao_fbo);
	
	ssao_shader_ = make_shared<SSAOShader>();
	// init kernel
	ssao_kernal_samples.resize(ssao_kernel_count);
	uniform_real_distribution<GLfloat> random_floats(0.0f, 1.0f);
	default_random_engine generator;
	auto my_lerp = [](GLfloat a, GLfloat b, GLfloat value)->float
	{
		return 	a + value*(b - a);
	};

	// 半球随机采样点数组
	for(unsigned int i = 0; i < ssao_kernel_count; i++)
	{
		vec3 sample(random_floats(generator) * 2.0 - 1.0,
			random_floats(generator) * 2.0 - 1.0,
			random_floats(generator));

		sample = normalize(sample) * random_floats(generator);
		float scale = (float)i / float(ssao_kernel_count);
		scale = my_lerp(0.1f, 1.0f, scale*scale);
		ssao_kernal_samples[i] = sample*scale;
	}

	// 引入一些随机旋转
	unsigned total_noise = ssao_noise_size*ssao_noise_size;
	ssao_kernal_noises.resize(total_noise);
	for(unsigned i = 0; i < total_noise; i++)
	{
		vec3 noise(random_floats(generator) * 2.0 - 1.0,
			random_floats(generator) * 2.0 - 1.0,
			0);
		ssao_kernal_noises[i] = noise;
	}

	ssao_shader_->Use();
	for(unsigned i = 0; i < ssao_kernel_count; ++i)
	{
		stringstream ss;
		ss << "samples[" << i << "]";
		ssao_shader_->SetVec3(ss.str(), ssao_kernal_samples[i]);
	}
	
	glGenTextures(1, &ssao_noise_texture);
	glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_kernal_noises[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// ssao模糊
	glGenFramebuffers(1, &SSAO_BlurFBO);
	
	// ssao blur shader
	map<EShaderType, string> blur_ssao_shader_path = {
		{vs, CSceneLoader::ToResourcePath("shader/ssao.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/ssao_blur.frag")},
	};
	
	ssao_blur_shader_ = make_shared<Shader>(blur_ssao_shader_path);
	ssao_blur_shader_->Use();
	ssao_blur_shader_->SetInt("ssao_texture", 0);

	GenerateSSAOTextures(width, height);
}

void SSAOHelper::GenerateSSAOTextures(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
	// 绑定对应的ssao计算结果贴图
	if(ssao_result_texture)
	{
		glDeleteTextures(1, &ssao_result_texture);
	}
	glGenTextures(1, &ssao_result_texture);
	glBindTexture(GL_TEXTURE_2D, ssao_result_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_result_texture, 0);

	// ssao模糊
	glBindFramebuffer(GL_FRAMEBUFFER, SSAO_BlurFBO);
	if(ssao_blur_texture)
	{
		glDeleteTextures(1, &ssao_blur_texture);
	}
	glGenTextures(1, &ssao_blur_texture);
	glBindTexture(GL_TEXTURE_2D, ssao_blur_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_texture, 0);

	ssao_shader_->Use();
	ssao_shader_->SetVec2("screen_size", vec2(width, height));
}


CRender* CRender::GetRender()
{
	return g_render;
}
GLuint CRender::GetNullTexId()
{
	return g_render->null_tex_id;
}

vec2 CRender::GetNearFar()
{
	return g_render->mainCamera->GetNearFar();
}

shared_ptr<CQuadShape> CRender::GetScreenShape()
{
	return g_render->quad_shape;
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
	quad_shape = make_shared<CQuadShape>();
	quad_shape->InitRenderInfo();
	m_SkyBox.Init();
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
	post_process.Init();
	glm::ivec2 window_size = Engine::GetEngine().GetWindowSize();
	int width = window_size.x;
	int height = window_size.y;
	
	defer_buffer_.Init(width, height);
	ssao_helper_.Init(width, height);

	// 屏幕空间反射shader
	ssreflection_shader = make_shared<SSReflectionShader>();
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

int CRender::Update(double delta)
{
	mainCamera->Update(delta);		
	// RenderSkyBox();
	CollectLightInfo();
	m_SkyBox.PreRenderUpdate();
	RenderShadowMap();			
	RenderSceneObject();
	post_process.RenderUI();
	post_process.SetPositionTexture(defer_buffer_.g_position_);	//先放这
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
#if !SHADOWMAP_DEBUG
	// 延迟渲染需要先关掉混合，否则混合操作可能会导致延迟渲染的各个参数贴图的a/w通道影响rgb/xyz值的情况
	glDisable(GL_BLEND);
	ivec2 window_size = Engine::GetEngine().GetWindowSize();
	

	glViewport(0,0, window_size.x, window_size.y);
#if USE_DERER_RENDER
	// 渲染到g buffer上
	glBindFramebuffer(GL_FRAMEBUFFER, defer_buffer_.g_buffer_);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DeferRenderSceneToGBuffer();
	if(use_ssao)
	{
		// 处理SSAO效果
		SSAORender();
	}
#endif
	
	// 渲染到后处理framebuffer上
	glBindFramebuffer(GL_FRAMEBUFFER, post_process.GetScreenFrameBuffer());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
#if USE_DERER_RENDER
	defer_buffer_.defer_render_shader->Use();
	defer_buffer_.defer_render_shader->SetBool("use_ssao", use_ssao);
	defer_buffer_.defer_render_shader->SetBool("use_rsm", use_rsm);
	defer_buffer_.defer_render_shader->SetFloat("rsm_intensity", rsm_intensity);
	glActiveTexture(GL_TEXTURE0 + 16);
	glBindTexture(GL_TEXTURE_2D, ssao_helper_.ssao_blur_texture);
	DeferRenderSceneLighting();
	
	// 需要将延迟渲染的深度缓冲复制到后面的后处理buffer上
	glBindFramebuffer(GL_READ_FRAMEBUFFER, defer_buffer_.g_buffer_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, post_process.GetScreenFrameBuffer());
	glBlitFramebuffer(0, 0, window_size.x, window_size.y, 0, 0, window_size.x, window_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RenderScene();
	RenderSkyBox();
	
	// screen space reflection先放在这里吧
	if(use_screen_space_reflection)
	{
		// 屏幕空间反射的信息渲染到后处理buffer的第三个color attachment贴图中，后通过后处理合成
		SSReflectionRender();
	}
	
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

void CRender::RenderSkyBox()
{
	if(render_sky_env_status == 0)
	{
		return;
	}
	
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrixNoTranslate(); //
	//mat4 mvp = projection * mainCamera->GetViewMatrix(); //
	m_SkyBox.Render(mvp, render_sky_env_status);
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
		auto mesh_shader = mesh_component->shader_data;
		// 跳过延迟渲染的mesh
		if(dynamic_pointer_cast<DeferInfoShader>(mesh_shader))
		{
			continue;
		}

		// 等于1代表渲染skybox，会需要用到环境贴图
		mesh_shader->Use();
		mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_shader->SetMat4("model", actor->GetModelMatrix());
		mesh_component->Draw(scene_render_info);
	}
}

void CRender::DeferRenderSceneToGBuffer() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	
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
		auto mesh_shader = mesh_component->shader_data;
		if(!dynamic_pointer_cast<DeferInfoShader>(mesh_shader))
		{
			continue;
		}
		
		// matrix_ubo.Bind();
		// matrix_ubo.UpdateData(actor->GetModelMatrix(), "model");
		// matrix_ubo.EndBind();

		mesh_shader->Use();
		// 等于1代表渲染skybox，会需要用到环境贴图
		mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_shader->SetMat4("model", actor->GetModelMatrix());
		mesh_component->Draw(scene_render_info);
	}
}

void CRender::DeferRenderSceneLighting() const
{
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);
	// glEnable(GL_DEPTH_TEST);
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
	
	// // 更新UBO里的矩阵数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(scene_render_info.camera_view, "view");
	matrix_ubo.UpdateData(scene_render_info.camera_proj, "projection");
	matrix_ubo.UpdateData(scene_render_info.camera_pos, "cam_pos");
	matrix_ubo.EndBind();
	
	defer_buffer_.defer_render_shader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_position_);

	// normal map加一个法线贴图的数据
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_normal_);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_albedo_);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_orm_);

	defer_buffer_.defer_render_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
	
	defer_buffer_.defer_render_shader->UpdateRenderData(quad_shape->mesh_resource->mesh_list[0].m_RenderInfo.material, scene_render_info);
	quad_shape->Draw();
}

void CRender::SSAORender() const
{
	// 处理SSAO效果
	glBindFramebuffer(GL_FRAMEBUFFER, ssao_helper_.ssao_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ssao_helper_.ssao_shader_->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_position_);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_normal_);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, ssao_helper_.ssao_noise_texture);

	// ssao_shader_->SetVec2("screen_size");
	// kernal samples to shader
	quad_shape->Draw();
	
	// blur
	glBindFramebuffer(GL_FRAMEBUFFER, ssao_helper_.SSAO_BlurFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	ssao_helper_.ssao_blur_shader_->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssao_helper_.ssao_result_texture);

	quad_shape->Draw();
}

void CRender::SSReflectionRender() const
{
	// scene color：post_process.screen_quad_texture[0]
	// scene normal：defer_buffer_.g_normal_
	// scene reflection mask: defer_buffer_.g_orm_
	// scene position: defer_buffer_.g_position_
	// scene depth存在于normal贴图的w分量上
	// glBindFramebuffer(GL_FRAMEBUFFER, ssao_helper_.ssao_fbo);
	// glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// glClear(GL_COLOR_BUFFER_BIT);

	ssreflection_shader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_position_);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_normal_);
	glActiveTexture(GL_TEXTURE0 + 2);
	// 用给后处理的texture作为scene color
	glBindTexture(GL_TEXTURE_2D, post_process.screen_quad_texture[0]);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_orm_);

	// ssao_shader_->SetVec2("screen_size");
	// kernal samples to shader
	quad_shape->Draw();
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

void CRender::OnWindowResize(int width, int height)
{
	post_process.OnWindowResize(width, height);
	defer_buffer_.GenerateDeferRenderTextures(width, height);
	ssao_helper_.GenerateSSAOTextures(width, height);
}
