/**
 * @file OpenGLRenderPassHost.cpp
 * @brief OpenGL 子系统与 Pass 注册宿主（原 RenderModuleOpenGLPasses + 部分 RenderModule GL 资源）。
 */

#ifndef RENDER_IN_VULKAN

#include "OpenGLRenderPassHost.hpp"

#include "Render/RenderModule.hpp"
#include "Render/Abstraction/IFrameContext.hpp"
#include "Render/Abstraction/IGraphicsDevice.hpp"
#include "Render/Abstraction/IRHICommandList.hpp"
#include "Render/Abstraction/RenderSystemAdapter.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Render/Resource/Texture.hpp"
#include "Window.hpp"
#include "common.h"

#include <imgui.h>

using namespace Kong;

void OpenGLRenderPassHost::OnAttached(KongRenderModule& module, IGraphicsDevice* device)
{
    (void)device;
    InitMainFBO();
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
    InitUBOForModule(module);
    RegisterDefaultRenderPasses(module);
}

void OpenGLRenderPassHost::InitMainFBO()
{
    auto window_size = KongWindow::GetWindowModule().windowSize;
    int  width        = window_size.x;
    int  height       = window_size.y;

    glGenFramebuffers(1, &m_renderToBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderToBuffer);

    TextureCreateInfo fragout_texture_create_info{
        GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT,
        width, height, GL_REPEAT, GL_REPEAT, GL_REPEAT,
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER};

    for (unsigned i = 0; i < FRAGOUT_TEXTURE_COUNT; ++i)
    {
        TextureBuilder::CreateTexture(m_renderToTextures[i], fragout_texture_create_info);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_renderToTextures[i], 0);
    }
    if (!m_renderToRbo)
        glGenRenderbuffers(1, &m_renderToRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderToRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderToRbo);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    GLuint color_attachment[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, color_attachment);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_mainSceneGLFramebuffer.SetGLName(m_renderToBuffer);
}

void OpenGLRenderPassHost::InitUBOForModule(KongRenderModule& module)
{
    m_matrix_ubo.AppendData(glm::mat4(), "view");
    m_matrix_ubo.AppendData(glm::mat4(), "projection");
    m_matrix_ubo.AppendData(glm::vec4(), "cam_pos");
    m_matrix_ubo.AppendData(glm::vec4(), "near_far");
    m_matrix_ubo.Init(0);

    m_scene_light_ubo.AppendData(SceneLightInfo(), "light_info");
    m_scene_light_ubo.Init(1);

    m_matrix_ubo.Bind();
    m_matrix_ubo.UpdateData(glm::vec4(module.GetCamera()->GetNearFar(), 0, 0), "near_far");
}

void OpenGLRenderPassHost::RegisterDefaultRenderPasses(KongRenderModule& module)
{
    m_glSkyboxRenderSystem.Init();
    m_glDeferRenderSystem.Init();
    m_glPostProcessRenderSystem.Init();
    m_glSSReflectionRenderSystem.Init();
    m_glWaterRenderSystem.Init();

    KongRenderModule* m    = &module;
    OpenGLRenderPassHost* h = this;

    module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([m](IFrameContext&, SceneDrawInfo& sd) {
        if (sd.rhiCommandList)
            sd.rhiCommandList->SetViewport(0.f, 0.f, static_cast<float>(SHADOW_RESOLUTION),
                                           static_cast<float>(SHADOW_RESOLUTION), 0.f, 1.f);
        m->RenderShadowMap();
    }));
    module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([m, h](IFrameContext&, SceneDrawInfo& sceneDrawInfo) {
        sceneDrawInfo.currentFramebuffer = h->GetMainSceneFramebuffer(*m);
        h->DrawMainScene(*m, sceneDrawInfo.rhiCommandList);
        sceneDrawInfo.currentColorRT = static_cast<uintptr_t>(m->latestRenderResult.resultColor);
        sceneDrawInfo.currentDepthRT = static_cast<uintptr_t>(m->latestRenderResult.resultDepth);
    }));
    module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([m, h](IFrameContext&, SceneDrawInfo& sceneDrawInfo) {
        RenderResultInfo rri;
        rri.frameBuffer   = m->latestRenderResult.frameBuffer;
        rri.resultColor   = static_cast<GLuint>(sceneDrawInfo.currentColorRT);
        rri.resultDepth   = static_cast<GLuint>(sceneDrawInfo.currentDepthRT);
        rri.resultBloom   = m->latestRenderResult.resultBloom;
        rri.resultPosition = m->latestRenderResult.resultPosition;
        m->latestRenderResult = h->GetWater().Draw(static_cast<double>(sceneDrawInfo.frameTime), rri, m);
        sceneDrawInfo.currentColorRT = static_cast<uintptr_t>(m->latestRenderResult.resultColor);
        sceneDrawInfo.currentDepthRT = static_cast<uintptr_t>(m->latestRenderResult.resultDepth);
    }));
    module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([m, h](IFrameContext&, SceneDrawInfo& sceneDrawInfo) {
        RenderResultInfo rri;
        rri.frameBuffer   = m->latestRenderResult.frameBuffer;
        rri.resultColor   = static_cast<GLuint>(sceneDrawInfo.currentColorRT);
        rri.resultDepth   = static_cast<GLuint>(sceneDrawInfo.currentDepthRT);
        rri.resultBloom   = m->latestRenderResult.resultBloom;
        rri.resultPosition = m->latestRenderResult.resultPosition;
        m->latestRenderResult = h->GetPostProcess().Draw(0.0, rri, m);
        sceneDrawInfo.currentColorRT = static_cast<uintptr_t>(m->latestRenderResult.resultColor);
        sceneDrawInfo.currentDepthRT = static_cast<uintptr_t>(m->latestRenderResult.resultDepth);
    }));
    module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([m, h](IFrameContext&, SceneDrawInfo&) {
        m->RenderUIBeforeSystems();
        h->DrawSubsystemUI(*m);
    }));
}

OpenGLRenderSystem* OpenGLRenderPassHost::TryGetOpenGLSubsystem(RenderSystemType type)
{
    switch (type)
    {
    case RenderSystemType::SKYBOX: return &m_glSkyboxRenderSystem;
    case RenderSystemType::DEFERRED: return &m_glDeferRenderSystem;
    case RenderSystemType::POST_PROCESS: return &m_glPostProcessRenderSystem;
    case RenderSystemType::SS_REFLECTION: return &m_glSSReflectionRenderSystem;
    default: return nullptr;
    }
}

IRHIRenderSubsystem* OpenGLRenderPassHost::TryGetRHISubsystem(RHISubsystemKind kind)
{
    switch (kind)
    {
    case RHISubsystemKind::Deferred: return &m_glDeferRenderSystem;
    case RHISubsystemKind::Skybox: return &m_glSkyboxRenderSystem;
    case RHISubsystemKind::PostProcess: return &m_glPostProcessRenderSystem;
    case RHISubsystemKind::ScreenSpaceReflection: return &m_glSSReflectionRenderSystem;
    case RHISubsystemKind::Water: return &m_glWaterRenderSystem;
    default: return nullptr;
    }
}

IFramebuffer* OpenGLRenderPassHost::GetMainSceneFramebuffer(KongRenderModule& module)
{
    (void)module;
    return &m_mainSceneGLFramebuffer;
}

void OpenGLRenderPassHost::OnSceneLightInfoUpdated(KongRenderModule& module, const SceneLightInfo& lightInfo)
{
    (void)module;
    m_scene_light_ubo.Bind();
    m_scene_light_ubo.UpdateData(lightInfo, "light_info");
    m_scene_light_ubo.EndBind();
}

void OpenGLRenderPassHost::DrawMainScene(KongRenderModule& module, IRHICommandList* rhiCommandList)
{
#if !SHADOWMAP_DEBUG
    m_glSkyboxRenderSystem.PreRenderUpdate();
    m_matrix_ubo.Bind();
    m_matrix_ubo.UpdateData(module.GetCamera()->GetViewMatrix(), "view");
    m_matrix_ubo.UpdateData(module.GetCamera()->GetProjectionMatrix(), "projection");
    m_matrix_ubo.UpdateData(module.GetCamera()->GetPosition(), "cam_pos");
    m_matrix_ubo.EndBind();

    glDisable(GL_BLEND);
    glm::ivec2 window_size = KongWindow::GetWindowModule().windowSize;
    GLuint     render_scene_buffer = m_renderToBuffer;
    RenderResultInfo render_result_info;
    render_result_info.frameBuffer = render_scene_buffer;
    render_result_info.resultColor = m_renderToTextures[0];
    render_result_info.resultDepth = module.latestRenderResult.resultDepth;
    module.latestRenderResult.frameBuffer   = render_scene_buffer;
    module.latestRenderResult.resultColor   = render_result_info.resultColor;
    module.latestRenderResult.rhiFramebuffer = &m_mainSceneGLFramebuffer;
    if (rhiCommandList)
    {
        rhiCommandList->SetViewport(0.f, 0.f, static_cast<float>(window_size.x), static_cast<float>(window_size.y),
                                   0.f, 1.f);
        rhiCommandList->BindFramebuffer(&m_mainSceneGLFramebuffer);
        const float clearZero[4] = {0.f, 0.f, 0.f, 0.f};
        rhiCommandList->ClearRenderTarget(RHIClearMask::Color | RHIClearMask::Depth, clearZero, 1.f, 0);
    }
    else
        glViewport(0, 0, window_size.x, window_size.y);

    render_result_info = m_glDeferRenderSystem.Draw(0.0, render_result_info, &module);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    module.RenderNonDeferSceneObjects(m_glSkyboxRenderSystem.render_sky_env_status);
    render_result_info = m_glSkyboxRenderSystem.Draw(0.0, render_result_info, &module);
    if (module.use_screen_space_reflection)
        render_result_info = m_glSSReflectionRenderSystem.Draw(0.0, render_result_info, &module);

    module.latestRenderResult = render_result_info;
#endif
}

void OpenGLRenderPassHost::DrawSubsystemUI(KongRenderModule& module)
{
    (void)module;
    m_glSkyboxRenderSystem.DrawUI();
    m_glDeferRenderSystem.DrawUI();
    m_glPostProcessRenderSystem.DrawUI();
}

void OpenGLRenderPassHost::DrawFallbackOpenGL(KongRenderModule& module, double delta)
{
    module.RenderShadowMap();
    DrawMainScene(module, nullptr);
    module.latestRenderResult =
        m_glWaterRenderSystem.Draw(delta, module.latestRenderResult, &module);
    module.latestRenderResult = m_glPostProcessRenderSystem.Draw(0.0, module.latestRenderResult, &module);
    module.RenderUIBeforeSystems();
    DrawSubsystemUI(module);
}

void OpenGLRenderPassHost::OnWindowResize(KongRenderModule& module, int width, int height)
{
    (void)module;
    m_glPostProcessRenderSystem.OnWindowResize(static_cast<unsigned>(width), static_cast<unsigned>(height));
}

void OpenGLRenderPassHost::SetRenderWater(KongRenderModule& module, const std::weak_ptr<AActor>& water)
{
    (void)module;
    m_glWaterRenderSystem.m_waterActor = water;
}

uint32_t OpenGLRenderPassHost::GetMainFBONativeHandle() const
{
    return static_cast<uint32_t>(m_renderToBuffer);
}

uint32_t OpenGLRenderPassHost::GetMainColorTextureNativeHandle(unsigned index) const
{
    return index < FRAGOUT_TEXTURE_COUNT ? static_cast<uint32_t>(m_renderToTextures[index]) : 0u;
}

GLuint OpenGLRenderPassHost::GetFragmentOutTexture(unsigned index) const
{
    return index < FRAGOUT_TEXTURE_COUNT ? m_renderToTextures[index] : GL_NONE;
}

#if SHADOWMAP_DEBUG
void OpenGLRenderPassHost::DrawShadowMapDebug(KongRenderModule& module)
{
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    auto window_size = KongWindow::GetWindowModule().windowSize;
    int  width        = window_size.x;
    int  height       = window_size.y;
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CDirectionalLightComponent* dir_light = module.scene_render_info.scene_dirlight.lock().get();
    if (!dir_light || !shadowmap_debug_shader)
        return;
    shadowmap_debug_shader->Use();
    glActiveTexture(GL_TEXTURE0);
    GLuint dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
#if USE_CSM
    glBindTexture(GL_TEXTURE_2D_ARRAY, dir_light_shadowmap_id);
#else
    glBindTexture(GL_TEXTURE_2D, dir_light_shadowmap_id);
#endif
    if (m_QuadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
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
}
#endif

#endif

