#include "DeferRenderSystem.hpp"

#include <imgui.h>
#include <random>

#include "Actor.hpp"
#include "RenderModule.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Window.hpp"

using namespace Kong;


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
	TextureCreateInfo ssao_noise_create_info{
	GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT, 4, 4
	};
	ssao_noise_create_info.data = ssao_kernal_noises.data();
	TextureBuilder::CreateTexture(ssao_noise_texture, ssao_noise_create_info);

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
	TextureCreateInfo ssao_tex_create_info {
	GL_TEXTURE_2D, GL_RED, GL_RGB, GL_FLOAT, width, height
	};

	TextureBuilder::CreateTexture(ssao_result_texture, ssao_tex_create_info);
	// 绑定对应的ssao计算结果贴图
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_result_texture, 0);

	// ssao模糊
	glBindFramebuffer(GL_FRAMEBUFFER, SSAO_BlurFBO);
	TextureBuilder::CreateTexture(ssao_blur_texture, ssao_tex_create_info);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_texture, 0);
	
	ssao_shader_->SetVec2("screen_size", vec2(width, height));
}

DeferRenderSystem::DeferRenderSystem()
{
	m_Type = RenderSystemType::DEFERRED;
}


void DeferRenderSystem::Init()
{
    glGenFramebuffers(1, &m_infoBuffer);

    auto& window_size = KongWindow::GetWindowModule().windowSize;
    // info buffer
    GenerateDeferInfoTextures(window_size.x, window_size.y);
    

	m_ssaoHelper.Init(window_size.x, window_size.y);
	
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

	// 绑定一些texture
	m_deferredBRDFShader = make_shared<DeferredBRDFShader>();
}

RenderResultInfo DeferRenderSystem::Draw(double delta, const RenderResultInfo& render_result_info,
    KongRenderModule* render_module)
{
    // 渲染到g buffer上
    glBindFramebuffer(GL_FRAMEBUFFER, m_infoBuffer);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 然后再渲染到目标
    RenderToBuffer(render_module);
    
    // 水面反射不做SSAO
    if(use_ssao)
    {
        // 处理SSAO效果
        SSAORender();
    }

    RenderToTexture(render_result_info.frameBuffer, render_module);
	
    return RenderResultInfo{render_result_info.frameBuffer,
        render_result_info.resultColor,
        GetNormalTexture(),
        render_result_info.resultBloom,
		GetPositionTexture()};
}

void DeferRenderSystem::DrawUI()
{
	ImGui::Checkbox("ssao", &use_ssao);
	
	ImGui::Checkbox("reflective shadowmap(rsm)", &use_rsm);
	ImGui::DragFloat("rsm intensity", &rsm_intensity, 0.005f, 0., 1.0);

	if (ImGui::TreeNode("Percentage-Closer Soft Shadows"))
	{
		ImGui::Checkbox("Use PCSS", &use_pcss);
		ImGui::DragFloat("PCSS radius", &pcss_radius, 0.01f, 0.1f, 50.0f);
		ImGui::DragFloat("PCSS light scale", &pcss_light_scale, 0.01f, 0.1f, 1.0f);
		ImGui::DragInt("PCSS sample count", &pcss_sample_count, 0.1f, 8, 64);
		ImGui::TreePop();
	}
}

GLuint DeferRenderSystem::GetPositionTexture()
{
    return m_deferredResourceMap[DeferResType::Position];
}

GLuint DeferRenderSystem::GetNormalTexture()
{
    return m_deferredResourceMap[DeferResType::Normal];
}

GLuint DeferRenderSystem::GetAlbedoTexture()
{
    return m_deferredResourceMap[DeferResType::Albedo];
}

GLuint DeferRenderSystem::GetOrmTexture()
{
    return m_deferredResourceMap[DeferResType::Orm];
}

GLuint DeferRenderSystem::GetFrameBuffer() const
{
    return m_infoBuffer;
}

void DeferRenderSystem::GenerateDeferInfoTextures(int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_infoBuffer);

    TextureCreateInfo defer_tex_create_info
    {
        GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT,
        width, height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_NEAREST, GL_NEAREST
    };
	
    // 将当前视野的数据用贴图缓存
    // 位置数据
    TextureBuilder::CreateTexture(m_deferredResourceMap[DeferResType::Position], defer_tex_create_info);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_deferredResourceMap[DeferResType::Position], 0);
	
    // 法线数据
    TextureBuilder::CreateTexture(m_deferredResourceMap[DeferResType::Normal], defer_tex_create_info);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_deferredResourceMap[DeferResType::Normal], 0);

    TextureCreateInfo albedo_tex_create_info {defer_tex_create_info};
    albedo_tex_create_info.internalFormat = GL_RGBA;
    albedo_tex_create_info.data_type = GL_UNSIGNED_BYTE;

    // 顶点颜色数据
    TextureBuilder::CreateTexture(m_deferredResourceMap[DeferResType::Albedo], albedo_tex_create_info);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_deferredResourceMap[DeferResType::Albedo], 0);

    // orm数据（ao，roughness，metallic）
    TextureBuilder::CreateTexture(m_deferredResourceMap[DeferResType::Orm], albedo_tex_create_info);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_deferredResourceMap[DeferResType::Orm], 0);

    if(m_infoRbo)
    {
        glDeleteRenderbuffers(1, &m_infoRbo);
    }
    // 生成renderbuffer
    glGenRenderbuffers(1, &m_infoRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_infoRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_infoRbo);
    glEnable(GL_DEPTH_TEST);
	
    unsigned int attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferRenderSystem::RenderToBuffer(KongRenderModule* render_module)
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
        if(!dynamic_pointer_cast<DeferInfoShader>(mesh_shader)
            && !dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader)
            )
        {
            continue;
        }

		auto skybox_sys = dynamic_cast<SkyboxRenderSystem*>(render_module->GetRenderSystemByType(RenderSystemType::SKYBOX));
    	
        mesh_shader->Use();
        // 等于1代表渲染skybox，会需要用到环境贴图
        mesh_shader->SetBool("b_render_skybox", skybox_sys->render_sky_env_status == 1);
        mesh_shader->SetMat4("model", actor->GetModelMatrix());
        mesh_component->Draw(render_module->scene_render_info);
    }
}

void DeferRenderSystem::RenderToTexture(GLuint render_to_buffer, KongRenderModule* render_module)
{
    glBindFramebuffer(GL_FRAMEBUFFER, render_to_buffer);
    // 渲染到当前的scene framebuffer上
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_deferredBRDFShader->Use();
    m_deferredBRDFShader->SetBool("use_ssao", use_ssao);
    // rsm
    m_deferredBRDFShader->SetBool("use_rsm", use_rsm);
    m_deferredBRDFShader->SetFloat("rsm_intensity", rsm_intensity);
    m_deferredBRDFShader->SetInt("rsm_sample_count", rsm_sample_count);
    for(int i = 0; i < rsm_sample_count; i++)
    {
        stringstream rsm_stream;
        rsm_stream <<  "rsm_samples_and_weights[" << i << "]";
        m_deferredBRDFShader->SetVec4(rsm_stream.str(), rsm_samples_and_weights[i]);
    }

    // pcss
    m_deferredBRDFShader->SetBool("use_pcss", use_pcss);
    m_deferredBRDFShader->SetFloat("pcss_radius", pcss_radius);
    m_deferredBRDFShader->SetFloat("pcss_light_scale", pcss_light_scale);
    m_deferredBRDFShader->SetInt("pcss_sample_count", pcss_sample_count);

    if (use_ssao)
    {
        glBindTextureUnit(16, m_ssaoHelper.ssao_blur_texture);
    }
	
    // 渲染光照
	int texture_idx = 0;
	glBindTextureUnit(texture_idx++, GetPositionTexture());
	glBindTextureUnit(texture_idx++, GetNormalTexture());
	glBindTextureUnit(texture_idx++, GetAlbedoTexture());
	glBindTextureUnit(texture_idx++, GetOrmTexture());
	
	auto skybox_sys = dynamic_cast<SkyboxRenderSystem*>(render_module->GetRenderSystemByType(RenderSystemType::SKYBOX));
    m_deferredBRDFShader->SetBool("b_render_skybox", skybox_sys->render_sky_env_status == 1);
	// auto& render_module = KongRenderModule::GetRenderModule();
	glBindTextureUnit(texture_idx++, skybox_sys->GetSkyBoxTextureId());
	glBindTextureUnit(texture_idx++, skybox_sys->GetDiffuseIrradianceTexture());
	glBindTextureUnit(texture_idx++, skybox_sys->GetPrefilterTexture());
	glBindTextureUnit(texture_idx++, skybox_sys->GetBRDFLutTexture());

	auto m_quadShape = KongRenderModule::GetScreenShape();
    m_deferredBRDFShader->UpdateRenderData(m_quadShape->mesh_resource->mesh_list[0].m_RenderInfo.material,
        render_module->scene_render_info);
	
    m_quadShape->Draw();
    
    // 需要将延迟渲染的深度缓冲复制到后面的后处理buffer上
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_infoBuffer);
    auto window_size = KongWindow::GetWindowModule().windowSize;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_to_buffer);
    glBlitFramebuffer(0, 0, window_size.x, window_size.y, 0, 0,
        window_size.x, window_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
}

void DeferRenderSystem::SSAORender()
{
	// 处理SSAO效果
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoHelper.ssao_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_ssaoHelper.ssao_shader_->Use();
	glBindTextureUnit(0, GetPositionTexture());
	glBindTextureUnit(1, GetNormalTexture());
	
	glBindTextureUnit(2, m_ssaoHelper.ssao_noise_texture);
	auto m_quadShape = KongRenderModule::GetScreenShape();
	// kernal samples to shader
	m_quadShape->Draw();
	
	// blur
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoHelper.SSAO_BlurFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	m_ssaoHelper.ssao_blur_shader_->Use();
	glBindTextureUnit(0, m_ssaoHelper.ssao_result_texture);

	m_quadShape->Draw();
}
