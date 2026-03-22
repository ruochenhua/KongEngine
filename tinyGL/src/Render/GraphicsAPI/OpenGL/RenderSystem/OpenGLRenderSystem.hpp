#pragma once
#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "Render/Abstraction/RenderSubsystemTypes.hpp"

namespace Kong
{
    class KongRenderModule;
    class ITexture;
    class IFramebuffer;

    /**
     * 返回渲染结果信息。
     * @deprecated GLuint 字段为 GL 遗留；新 Pass 应优先写入 rhiFramebuffer / rhiResultColor / rhiResultDepth。
     */
    struct RenderResultInfo
    {
        GLuint frameBuffer {GL_NONE};
        GLuint resultColor {GL_NONE};
        GLuint resultDepth {GL_NONE};
        GLuint resultBloom {GL_NONE};
        GLuint resultPosition {GL_NONE};
        IFramebuffer* rhiFramebuffer = nullptr;
        ITexture*     rhiResultColor   = nullptr;
        ITexture*     rhiResultDepth   = nullptr;
    };

    // 渲染传入信息
    struct RenderInputInfo
    {
        GLuint frameBuffer {GL_NONE};
    };

    // 渲染系统，每个渲染效果或者阶段都独立出来
    class OpenGLRenderSystem
    {
    public:
        OpenGLRenderSystem() = default;
        virtual ~OpenGLRenderSystem() = default;

        // 禁止system之间的复制操作
        OpenGLRenderSystem(const OpenGLRenderSystem&) = delete;
        OpenGLRenderSystem& operator=(const OpenGLRenderSystem&) = delete;
        
        virtual RenderResultInfo Draw(
            double delta,
            const RenderResultInfo& render_result_info,
            KongRenderModule* render_module)
        {
            return RenderResultInfo {};
        }
        virtual void DrawUI() {}
        virtual void Init() {}
        
    protected:
        GLuint m_frameBuffer {GL_NONE};
        RenderSystemType m_Type {RenderSystemType::NONE};
    };
}
