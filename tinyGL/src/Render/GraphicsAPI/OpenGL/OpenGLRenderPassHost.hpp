#pragma once

#include "Render/Abstraction/IRenderPassHost.hpp"
#include "Render/GraphicsAPI/OpenGL/GLFramebuffer.hpp"
#include "Render/GraphicsAPI/OpenGL/UBOHelper.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlDeferRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlSkyboxRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlPostProcessRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlSSReflectionRenderSystem.hpp"
#include "Render/GraphicsAPI/OpenGL/RenderSystem/GlWaterRenderSystem.hpp"
#include "Render/RenderCommon.hpp"
#include "Shader/OpenGL/OpenGLShader.h"

namespace Kong
{
    /**
     * OpenGL：持有 Gl*、主场景 FBO、矩阵/光照 UBO，并向 KongRenderModule 注册 Pass。
     */
    class OpenGLRenderPassHost final : public IRenderPassHost
    {
    public:
        OpenGLRenderPassHost() = default;
        ~OpenGLRenderPassHost() override = default;

        void OnAttached(KongRenderModule& module, IGraphicsDevice* device) override;
        void RegisterDefaultRenderPasses(KongRenderModule& module) override;

        OpenGLRenderSystem* TryGetOpenGLSubsystem(RenderSystemType type) override;
        IRHIRenderSubsystem* TryGetRHISubsystem(RHISubsystemKind kind) override;
        IFramebuffer* GetMainSceneFramebuffer(KongRenderModule& module) override;
        void OnSceneLightInfoUpdated(KongRenderModule& module, const SceneLightInfo& lightInfo) override;
        void DrawMainScene(KongRenderModule& module, IRHICommandList* cmd) override;
        void DrawSubsystemUI(KongRenderModule& module) override;
        void DrawFallbackOpenGL(KongRenderModule& module, double delta) override;
        void OnWindowResize(KongRenderModule& module, int width, int height) override;
        void SetRenderWater(KongRenderModule& module, const std::weak_ptr<AActor>& water) override;

        uint32_t GetMainFBONativeHandle() const override;
        uint32_t GetMainColorTextureNativeHandle(unsigned index) const override;

        bool HasSubsystemUI() const override { return true; }

        /** 供 GlPostProcess 等读取 MRT 纹理 ID（与旧 m_renderToTextures 等价） */
        GLuint GetFragmentOutTexture(unsigned index) const;

#if SHADOWMAP_DEBUG
        void DrawShadowMapDebug(KongRenderModule& module);
#endif

        void InitMainFBO();
        void InitUBOForModule(KongRenderModule& module);

        GlSkyboxRenderSystem&       GetSkybox() { return m_glSkyboxRenderSystem; }
        GlDeferRenderSystem&        GetDefer() { return m_glDeferRenderSystem; }
        GlPostProcessRenderSystem&  GetPostProcess() { return m_glPostProcessRenderSystem; }
        GlSSReflectionRenderSystem& GetSSReflection() { return m_glSSReflectionRenderSystem; }
        GlWaterRenderSystem&        GetWater() { return m_glWaterRenderSystem; }

        UBOHelper& GetMatrixUbo() { return m_matrix_ubo; }

    private:
        GlSkyboxRenderSystem       m_glSkyboxRenderSystem;
        GlDeferRenderSystem        m_glDeferRenderSystem;
        GlPostProcessRenderSystem  m_glPostProcessRenderSystem;
        GlSSReflectionRenderSystem m_glSSReflectionRenderSystem;
        GlWaterRenderSystem        m_glWaterRenderSystem;
        GLFramebuffer              m_mainSceneGLFramebuffer;

        GLuint m_renderToTextures[FRAGOUT_TEXTURE_COUNT] = {0, 0, 0};
        GLuint m_renderToBuffer {0};
        GLuint m_renderToRbo {0};

        UBOHelper m_matrix_ubo;
        UBOHelper m_scene_light_ubo;

        shared_ptr<OpenGLShader> shadowmap_debug_shader;
#if SHADOWMAP_DEBUG
        GLuint m_QuadVAO = GL_NONE;
        GLuint m_QuadVBO = GL_NONE;
#endif
    };
}
