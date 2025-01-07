#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include <random>
#include <utility>

#include "Actor.h"
#include "Component/CameraComponent.h"
#include "Engine.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "Scene.h"
#include "Shader/Shader.h"
#include "stb_image.h"
#include "Component/Mesh/GerstnerWaveWater.h"
#include "Component/Mesh/QuadShape.h"
#include "Component/Mesh/Water.h"
#include "glm/gtx/dual_quaternion.hpp"

using namespace Kong;
using namespace glm;
using namespace std;
#define DEFER_TERRAIN true

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


void WaterRenderHelper::Init(int width, int height)
{
	// water相关的buffer初始化
	glGenFramebuffers(1, &water_reflection_fbo);
	glGenFramebuffers(1, &water_refraction_fbo);

	GenerateWaterRenderTextures(width, height);
}

void WaterRenderHelper::GenerateWaterRenderTextures(int width, int height)
{
	// 水面反射相关的buffer
	glBindFramebuffer(GL_FRAMEBUFFER, water_reflection_fbo);
	
	if(water_reflection_texture)
	{
		glDeleteTextures(1, &water_reflection_texture);
	}
	
	glGenTextures(1, &water_reflection_texture);
	glBindTexture(GL_TEXTURE_2D, water_reflection_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_reflection_texture, 0);

	if (!water_reflection_rbo)
	{
		glGenRenderbuffers(1, &water_reflection_rbo);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, water_reflection_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, water_reflection_rbo);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("water reflection framebuffer error\n");
	}

	// 水面折射相关的buffer
	glBindFramebuffer(GL_FRAMEBUFFER, water_refraction_fbo);
	
	if(water_refraction_texture)
	{
		glDeleteTextures(1, &water_refraction_texture);
	}
	
	glGenTextures(1, &water_refraction_texture);
	glBindTexture(GL_TEXTURE_2D, water_refraction_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_refraction_texture, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("water refraction framebuffer error\n");
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	// quad_shape->InitRenderInfo();
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
	water_render_helper_.Init(width, height);

	// 屏幕空间反射shader
	ssreflection_shader = make_shared<SSReflectionShader>();

	// rsm采样点初始化
	std::default_random_engine random_engine;
	std::uniform_real_distribution<float> u(0.0, 1.0);
	float pi_num = pi<float>();
	for(int i = 0; i < rsm_sample_count; i++)
	{
		float xi1 = u(random_engine);
		float xi2 = u(random_engine);
		
		rsm_samples_and_weights.emplace_back(xi1*sin(2*pi_num*xi2), xi1*cos(2*pi_num*xi2), xi1*xi1, 0.0);
	}
	return 0;
}

int CRender::InitCamera()
{
	mainCamera = new CCamera(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));

	mainCamera->InitControl();
	return 0;
}

void CRender::UpdateSceneRenderInfo()
{
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
	render_time += delta;
	// 更新光照
	// todo: 这两个合起来
	CollectLightInfo();
	UpdateSceneRenderInfo();
	
	m_SkyBox.PreRenderUpdate();
	RenderShadowMap();
	// 更新UBO里的相机数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(mainCamera->GetViewMatrix(), "view");
	matrix_ubo.UpdateData(mainCamera->GetProjectionMatrix(), "projection");
	matrix_ubo.UpdateData(mainCamera->GetPosition(), "cam_pos");
	matrix_ubo.EndBind();
	
	// 普通渲染场景
	RenderSceneObject(false);

	// 有水体，需要做一次从下往上的渲染获取反射的内容
	if (water_render_helper_.water_actor.lock())
	{
		auto water_actor = water_render_helper_.water_actor.lock();
		
		vec3 origin_cam_pos = mainCamera->GetPosition();
		// camera在水面之上
		float height_diff = origin_cam_pos.y - water_actor->location.y;
		if (height_diff > 0.0f)
		{
			// 复制普通场景渲染中postprocess的scene texture作为折射贴图使用
			ivec2 window_size = Engine::GetEngine().GetWindowSize();
			glBindFramebuffer(GL_READ_FRAMEBUFFER, post_process.GetScreenFrameBuffer());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, water_render_helper_.water_refraction_fbo);
			glBlitFramebuffer(0, 0, window_size.x, window_size.y, 0, 0,
				window_size.x, window_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
			// 处理水面折射
			mainCamera->SetPosition(origin_cam_pos + vec3(0,-2*height_diff, 0));
			mainCamera->InvertPitch();
		
			matrix_ubo.Bind();
			matrix_ubo.UpdateData(mainCamera->GetViewMatrix(), "view");
			matrix_ubo.UpdateData(mainCamera->GetProjectionMatrix(), "projection");
			matrix_ubo.UpdateData(mainCamera->GetPosition(), "cam_pos");
			matrix_ubo.EndBind();
			RenderSceneObject(true);
			
			// // 渲染水面mesh
			glBindFramebuffer(GL_FRAMEBUFFER, post_process.GetScreenFrameBuffer());
		
			// 渲染水需要恢复相机位置
			mainCamera->SetPosition(origin_cam_pos);
			mainCamera->InvertPitch();
		
			matrix_ubo.Bind();
			matrix_ubo.UpdateData(mainCamera->GetViewMatrix(), "view");
			matrix_ubo.UpdateData(mainCamera->GetProjectionMatrix(), "projection");
			matrix_ubo.UpdateData(mainCamera->GetPosition(), "cam_pos");
			matrix_ubo.EndBind();
			
			RenderWater();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else
		{
			// todo: 水下
		}
	}

	// do post process
	DoPostProcess();

	
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

void CRender::DoPostProcess()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	post_process.Draw();
}

void CRender::RenderSceneObject(bool water_reflection)
{
#if !SHADOWMAP_DEBUG
	// 延迟渲染需要先关掉混合，否则混合操作可能会导致延迟渲染的各个参数贴图的a/w通道影响rgb/xyz值的情况
	glDisable(GL_BLEND);
	ivec2 window_size = Engine::GetEngine().GetWindowSize();

	// render_scene_texture是场景渲染到屏幕上的未经过后处理的结果
	
	GLuint render_scene_buffer;
	// 正常渲染到后处理的buffer上
	if (!water_reflection)
	{
		render_scene_buffer = post_process.GetScreenFrameBuffer();
	}
	else
	{
		// 水体反射的渲染到water scene fbo上
		render_scene_buffer = water_render_helper_.water_reflection_fbo;
	}
	
	glViewport(0,0, window_size.x, window_size.y);

	// 渲染到g buffer上
	glBindFramebuffer(GL_FRAMEBUFFER, defer_buffer_.g_buffer_);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DeferRenderSceneToGBuffer();
	// 水面反射不做SSAO
	if(use_ssao && !water_reflection)
	{
		// 处理SSAO效果
		SSAORender();
	}
	
	// 渲染到当前的scene framebuffer上
	glBindFramebuffer(GL_FRAMEBUFFER, render_scene_buffer);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	defer_buffer_.defer_render_shader->Use();
	defer_buffer_.defer_render_shader->SetBool("use_ssao", use_ssao);
	// rsm
	defer_buffer_.defer_render_shader->SetBool("use_rsm", use_rsm);
	defer_buffer_.defer_render_shader->SetFloat("rsm_intensity", rsm_intensity);
	defer_buffer_.defer_render_shader->SetInt("rsm_sample_count", rsm_sample_count);
	for(int i = 0; i < rsm_sample_count; i++)
	{
		stringstream rsm_stream;
		rsm_stream <<  "rsm_samples_and_weights[" << i << "]";
		defer_buffer_.defer_render_shader->SetVec4(rsm_stream.str(), rsm_samples_and_weights[i]);
	}

	// pcss
	defer_buffer_.defer_render_shader->SetBool("use_pcss", use_pcss);
	defer_buffer_.defer_render_shader->SetFloat("pcss_radius", pcss_radius);
	defer_buffer_.defer_render_shader->SetFloat("pcss_light_scale", pcss_light_scale);
	defer_buffer_.defer_render_shader->SetInt("pcss_sample_count", pcss_sample_count);

	if (use_ssao)
	{
		glActiveTexture(GL_TEXTURE0 + 16);
		glBindTexture(GL_TEXTURE_2D, ssao_helper_.ssao_blur_texture);
	}
	
	// 渲染光照
	DeferRenderSceneLighting();
	// 需要将延迟渲染的深度缓冲复制到后面的后处理buffer上
	glBindFramebuffer(GL_READ_FRAMEBUFFER, defer_buffer_.g_buffer_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_scene_buffer);
	glBlitFramebuffer(0, 0, window_size.x, window_size.y, 0, 0,
		window_size.x, window_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	RenderNonDeferSceneObjects();
	RenderSkyBox(defer_buffer_.g_normal_);
	
	// screen space reflection先放在这里吧
	// 水面反射不做这个
	if(use_screen_space_reflection && !water_reflection)
	{
		// 屏幕空间反射的信息渲染到后处理buffer的第三个color attachment贴图中，后通过后处理合成
		SSReflectionRender();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void CRender::ChangeSkybox()
{
	m_SkyBox.ChangeSkybox();
}

void CRender::RenderSkyBox(GLuint depth_texture)
{
	if(render_sky_env_status == 0)
	{
		return;
	}
	
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrixNoTranslate(); //
	//mat4 mvp = projection * mainCamera->GetViewMatrix(); //

	m_SkyBox.Render(mvp, render_sky_env_status, depth_texture);
}

void CRender::RenderNonDeferSceneObjects() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	
	auto actors = CScene::GetActors();
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
		mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_shader->SetMat4("model", actor->GetModelMatrix());
		mesh_shader->SetDouble("iTime", render_time);
		mesh_component->Draw(scene_render_info);
	}
}

void CRender::DeferRenderSceneToGBuffer() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	
	auto actors = CScene::GetActors();
	for(auto actor : actors)
	{
		auto mesh_component = actor->GetComponent<CMeshComponent>();
		if(!mesh_component)
		{
			continue;
		}
		auto mesh_shader = mesh_component->shader_data;
		if(!dynamic_pointer_cast<DeferInfoShader>(mesh_shader)
#if DEFER_TERRAIN
			&& !dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader)
#endif
			)
		{
			continue;
		}

		mesh_shader->Use();
		// 等于1代表渲染skybox，会需要用到环境贴图
		mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_shader->SetMat4("model", actor->GetModelMatrix());
		mesh_component->Draw(scene_render_info);
	}
}

void CRender::DeferRenderSceneLighting() const
{
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
	
	defer_buffer_.defer_render_shader->UpdateRenderData(quad_shape->mesh_resource->mesh_list[0].m_RenderInfo.material,
		scene_render_info);
	quad_shape->Draw();
}

void CRender::RenderWater()
{
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	water_render_helper_.total_move = render_time;
	if (water_render_helper_.water_actor.expired())
	{
		return;
	}

	shared_ptr<AActor> water_actor = water_render_helper_.water_actor.lock();
	
	// glEnable(GL_CLIP_DISTANCE0);
	// auto actors = CScene::GetActors();
	//for(auto actor : actors)
	auto water_comp = water_actor->GetComponent<Water>();
	if(water_comp)
	{
		auto mesh_shader = water_comp->shader_data;

		mesh_shader->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, water_render_helper_.water_reflection_texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, water_render_helper_.water_refraction_texture);
		
		// 等于1代表渲染skybox，会需要用到环境贴图
		// mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_shader->SetMat4("model", water_actor->GetModelMatrix());
		mesh_shader->SetFloat("move_factor",
			fmodf(water_render_helper_.total_move*water_render_helper_.move_speed, 1.0));
		water_comp->Draw(scene_render_info);
	}
	else
	{
		auto gerstner_water = water_actor->GetComponent<GerstnerWaveWater>();
		if(!gerstner_water)
		{
			return;
		}

		auto mesh_shader = gerstner_water->shader_data;

		mesh_shader->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, water_render_helper_.water_reflection_texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, water_render_helper_.water_refraction_texture);
		
		// 等于1代表渲染skybox，会需要用到环境贴图
		// mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_shader->SetMat4("model", water_actor->GetModelMatrix());
		mesh_shader->SetDouble("iTime", render_time);
		gerstner_water->Draw(scene_render_info);
	}
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
	// scene color：post_process.GetScreenTexture()
	// scene normal：defer_buffer_.g_normal_
	// scene reflection mask: defer_buffer_.g_orm_
	// scene position: defer_buffer_.g_position_
	// scene depth存在于normal贴图的w分量上

	// 这里要关掉深度测试，否则会影响后面的水体渲染的流程
	glDisable(GL_DEPTH_TEST);
	
	ssreflection_shader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_position_);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_normal_);
	glActiveTexture(GL_TEXTURE0 + 2);
	// 用给后处理的texture作为scene color
	glBindTexture(GL_TEXTURE_2D, post_process.GetScreenTexture());
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_orm_);
	
	quad_shape->Draw();
	glEnable(GL_DEPTH_TEST);
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
	water_render_helper_.GenerateWaterRenderTextures(width, height);
}

void CRender::SetRenderWater(const weak_ptr<AActor>& render_water_actor)
{
	water_render_helper_.water_actor = render_water_actor;
}
