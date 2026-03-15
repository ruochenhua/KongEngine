/**
 * @file RenderModuleBackendOpenGL.hpp
 * @brief OpenGL 后端：持有各 Gl* RenderSystem，注册 Shadow / 主场景 / Water / Postprocess / UI 等 Pass 适配器。
 */

#pragma once

#ifndef RENDER_IN_VULKAN

#include "Render/Abstraction/IRenderModuleBackend.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/OpenGLRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlDeferRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlSkyboxRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlPostProcessRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlSSReflectionRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlWaterRenderSystem.hpp"

namespace Kong
{
    class RenderModuleBackendOpenGL : public IRenderModuleBackend
    {
    public:
        void Init(KongRenderModule* module, IGraphicsDevice* device) override;
        void UpdateSceneRenderInfo(KongRenderModule* module) override;
        void OnReloadScene(KongRenderModule* module) override;
        void DrawFallback(KongRenderModule* module, double delta) override;
        void DrawMainScene(KongRenderModule* module) override;
        void DrawUI(KongRenderModule* module) override;
        void OnWindowResize(int width, int height) override;
        void SetRenderWater(const std::weak_ptr<AActor>& actor) override;

        /** 供 KongRenderModule::GetRenderSystemByType 及场景配置使用 */
        OpenGLRenderSystem* GetRenderSystemByType(RenderSystemType type);
        GlSkyboxRenderSystem* GetSkyboxRenderSystem() { return &m_skyboxRenderSystem; }
        GlDeferRenderSystem* GetDeferRenderSystem() { return &m_deferRenderSystem; }
        GlPostProcessRenderSystem* GetPostProcessRenderSystem() { return &m_postProcessRenderSystem; }
        GlSSReflectionRenderSystem* GetSSReflectionRenderSystem() { return &m_ssReflectionRenderSystem; }
        GlWaterRenderSystem* GetWaterRenderSystem() { return &m_waterRenderSystem; }

    private:
        GlSkyboxRenderSystem m_skyboxRenderSystem;
        GlDeferRenderSystem m_deferRenderSystem;
        GlPostProcessRenderSystem m_postProcessRenderSystem;
        GlSSReflectionRenderSystem m_ssReflectionRenderSystem;
        GlWaterRenderSystem m_waterRenderSystem;
    };
}

#endif
