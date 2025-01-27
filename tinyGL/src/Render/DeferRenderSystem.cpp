#include "DeferRenderSystem.hpp"

#include "Actor.hpp"
#include "RenderModule.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Window.hpp"

using namespace Kong;

void DeferRenderSystem::Init()
{
    m_quadShape = make_shared<CQuadShape>();
    
    glGenFramebuffers(1, &m_infoBuffer);

    auto& window_size = KongWindow::GetWindowModule().windowSize;
    // info buffer
    GenerateDeferInfoTextures(window_size.x, window_size.y);

    // render to buffer
    glGenFramebuffers(1, &m_renderToBuffer);
    GenerateDeferRenderToTextures(window_size.x, window_size.y);
    
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
    if(render_module->use_ssao)
    {
        // 处理SSAO效果
        // SSAORender();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, render_result_info.frameBuffer);
    RenderToTexture(render_result_info.frameBuffer, render_module);
    
    
    return RenderResultInfo{m_renderToBuffer,
        m_renderToTextures[0],
        GetNormalTexture(),
        m_renderToTextures[1]};
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

void DeferRenderSystem::GenerateDeferRenderToTextures(int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderToBuffer);

    TextureCreateInfo pp_texture_create_info
    {
        GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT,
        width, height, GL_REPEAT, GL_REPEAT, GL_REPEAT,
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    };

    
    for(unsigned i = 0; i < PP_TEXTURE_COUNT; ++i)
    {
        TextureBuilder::CreateTexture(m_renderToTextures[i], pp_texture_create_info);
        
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

void DeferRenderSystem::RenderToBuffer(const KongRenderModule* render_module) const
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

        mesh_shader->Use();
        // 等于1代表渲染skybox，会需要用到环境贴图
        mesh_shader->SetBool("b_render_skybox", render_module->render_sky_env_status == 1);
        mesh_shader->SetMat4("model", actor->GetModelMatrix());
        mesh_component->Draw(render_module->scene_render_info);
    }
}

void DeferRenderSystem::RenderToTexture(GLuint render_to_buffer,const Kong::KongRenderModule* render_module)
{
    
    // 渲染到当前的scene framebuffer上
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_deferredBRDFShader->Use();
    m_deferredBRDFShader->SetBool("use_ssao", render_module->use_ssao);
    // rsm
    m_deferredBRDFShader->SetBool("use_rsm", render_module->use_rsm);
    m_deferredBRDFShader->SetFloat("rsm_intensity", render_module->rsm_intensity);
    m_deferredBRDFShader->SetInt("rsm_sample_count", render_module->rsm_sample_count);
    for(int i = 0; i < render_module->rsm_sample_count; i++)
    {
        stringstream rsm_stream;
        rsm_stream <<  "rsm_samples_and_weights[" << i << "]";
        m_deferredBRDFShader->SetVec4(rsm_stream.str(), render_module->rsm_samples_and_weights[i]);
    }

    // pcss
    m_deferredBRDFShader->SetBool("use_pcss", render_module->use_pcss);
    m_deferredBRDFShader->SetFloat("pcss_radius", render_module->pcss_radius);
    m_deferredBRDFShader->SetFloat("pcss_light_scale", render_module->pcss_light_scale);
    m_deferredBRDFShader->SetInt("pcss_sample_count", render_module->pcss_sample_count);

    // if (use_ssao)
    // {
    //     glBindTextureUnit(16, ssao_helper_.ssao_blur_texture);
    //
    // }
	
    // 渲染光照
    m_deferredBRDFShader->Use();
	
    glBindTextureUnit(0, GetPositionTexture());
    glBindTextureUnit(1, GetNormalTexture());
    glBindTextureUnit(2, GetAlbedoTexture());
    glBindTextureUnit(3, GetOrmTexture());
	
    m_deferredBRDFShader->SetBool("b_render_skybox", render_module->render_sky_env_status == 1);
	
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
