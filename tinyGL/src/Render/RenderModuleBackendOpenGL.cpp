/**
 * @file RenderModuleBackendOpenGL.cpp
 * @brief OpenGL 后端：持有各 Gl* RenderSystem，注册 Shadow / 主场景 / Water / Postprocess / UI 适配器。
 * 仅在不定义 RENDER_IN_VULKAN 时参与编译。
 */

#ifndef RENDER_IN_VULKAN

#include "Render/RenderModuleBackendOpenGL.hpp"
#include "Render/RenderModule.hpp"
#include "Render/Abstraction/RenderSystemAdapter.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Window.hpp"

namespace Kong
{
    void RenderModuleBackendOpenGL::Init(KongRenderModule* module, IGraphicsDevice* device)
    {
        (void)device;
        m_skyboxRenderSystem.Init();
        m_deferRenderSystem.Init();
        m_postProcessRenderSystem.Init();
        m_ssReflectionRenderSystem.Init();
        m_waterRenderSystem.Init();

        RenderModuleBackendOpenGL* self = this;
        KongRenderModule* m = module;
        m->PushRenderSystem(std::make_unique<RenderSystemAdapter>([m](IFrameContext&, SceneDrawInfo&) {
            m->RenderShadowMap();
        }));
        m->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self, m](IFrameContext&, SceneDrawInfo& sceneDrawInfo) {
            self->DrawMainScene(m);
            sceneDrawInfo.currentColorRT = static_cast<uintptr_t>(m->latestRenderResult.resultColor);
            sceneDrawInfo.currentDepthRT = static_cast<uintptr_t>(m->latestRenderResult.resultDepth);
        }));
        m->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self, m](IFrameContext&, SceneDrawInfo& sceneDrawInfo) {
            RenderResultInfo rri;
            rri.frameBuffer = m->latestRenderResult.frameBuffer;
            rri.resultColor = static_cast<GLuint>(sceneDrawInfo.currentColorRT);
            rri.resultDepth = static_cast<GLuint>(sceneDrawInfo.currentDepthRT);
            rri.resultBloom = m->latestRenderResult.resultBloom;
            rri.resultPosition = m->latestRenderResult.resultPosition;
            m->latestRenderResult = self->m_waterRenderSystem.Draw(static_cast<double>(sceneDrawInfo.frameTime), rri, m);
            sceneDrawInfo.currentColorRT = static_cast<uintptr_t>(m->latestRenderResult.resultColor);
            sceneDrawInfo.currentDepthRT = static_cast<uintptr_t>(m->latestRenderResult.resultDepth);
        }));
        m->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self, m](IFrameContext&, SceneDrawInfo& sceneDrawInfo) {
            RenderResultInfo rri;
            rri.frameBuffer = m->latestRenderResult.frameBuffer;
            rri.resultColor = static_cast<GLuint>(sceneDrawInfo.currentColorRT);
            rri.resultDepth = static_cast<GLuint>(sceneDrawInfo.currentDepthRT);
            rri.resultBloom = m->latestRenderResult.resultBloom;
            rri.resultPosition = m->latestRenderResult.resultPosition;
            m->latestRenderResult = self->m_postProcessRenderSystem.Draw(0.0, rri, m);
            sceneDrawInfo.currentColorRT = static_cast<uintptr_t>(m->latestRenderResult.resultColor);
            sceneDrawInfo.currentDepthRT = static_cast<uintptr_t>(m->latestRenderResult.resultDepth);
        }));
        m->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self, m](IFrameContext&, SceneDrawInfo& sceneDrawInfo) {
            m->RenderUIBeforeSystems();
            self->DrawUI(m);
        }));
    }

    void RenderModuleBackendOpenGL::UpdateSceneRenderInfo(KongRenderModule* module)
    {
        (void)module;
    }

    void RenderModuleBackendOpenGL::OnReloadScene(KongRenderModule* module)
    {
        (void)module;
    }

    void RenderModuleBackendOpenGL::DrawFallback(KongRenderModule* module, double delta)
    {
        module->RenderShadowMap();
        DrawMainScene(module);
        module->latestRenderResult = m_waterRenderSystem.Draw(delta, module->latestRenderResult, module);
        module->latestRenderResult = m_postProcessRenderSystem.Draw(0.0, module->latestRenderResult, module);
        module->RenderUIBeforeSystems();
        DrawUI(module);
    }

    void RenderModuleBackendOpenGL::DrawMainScene(KongRenderModule* module)
    {
#if !SHADOWMAP_DEBUG
        m_skyboxRenderSystem.PreRenderUpdate();
        module->matrix_ubo.Bind();
        module->matrix_ubo.UpdateData(module->mainCamera->GetViewMatrix(), "view");
        module->matrix_ubo.UpdateData(module->mainCamera->GetProjectionMatrix(), "projection");
        module->matrix_ubo.UpdateData(module->mainCamera->GetPosition(), "cam_pos");
        module->matrix_ubo.EndBind();

        glDisable(GL_BLEND);
        glm::ivec2 window_size = KongWindow::GetWindowModule().windowSize;
        GLuint render_scene_buffer = module->GetMainFBO();
        RenderResultInfo render_result_info;
        render_result_info.frameBuffer = render_scene_buffer;
        render_result_info.resultColor = module->GetMainColorTexture(0);
        render_result_info.resultDepth = module->latestRenderResult.resultDepth;
        module->latestRenderResult.frameBuffer = render_scene_buffer;
        module->latestRenderResult.resultColor = render_result_info.resultColor;
        glViewport(0, 0, window_size.x, window_size.y);

        render_result_info = m_deferRenderSystem.Draw(0.0, render_result_info, module);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        module->RenderNonDeferSceneObjects(m_skyboxRenderSystem.render_sky_env_status);
        render_result_info = m_skyboxRenderSystem.Draw(0.0, render_result_info, module);
        if (module->use_screen_space_reflection)
            render_result_info = m_ssReflectionRenderSystem.Draw(0.0, render_result_info, module);

        module->latestRenderResult = render_result_info;
#endif
    }

    void RenderModuleBackendOpenGL::DrawUI(KongRenderModule* module)
    {
        (void)module;
        m_skyboxRenderSystem.DrawUI();
        m_deferRenderSystem.DrawUI();
        m_postProcessRenderSystem.DrawUI();
    }

    void RenderModuleBackendOpenGL::OnWindowResize(int width, int height)
    {
        m_postProcessRenderSystem.OnWindowResize(static_cast<unsigned>(width), static_cast<unsigned>(height));
    }

    void RenderModuleBackendOpenGL::SetRenderWater(const std::weak_ptr<AActor>& actor)
    {
        m_waterRenderSystem.m_waterActor = actor;
    }

    OpenGLRenderSystem* RenderModuleBackendOpenGL::GetRenderSystemByType(RenderSystemType type)
    {
        switch (type)
        {
        case RenderSystemType::SKYBOX:       return &m_skyboxRenderSystem;
        case RenderSystemType::DEFERRED:     return &m_deferRenderSystem;
        case RenderSystemType::POST_PROCESS: return &m_postProcessRenderSystem;
        case RenderSystemType::SS_REFLECTION: return &m_ssReflectionRenderSystem;
        default:                             return nullptr;
        }
    }
}

#endif
