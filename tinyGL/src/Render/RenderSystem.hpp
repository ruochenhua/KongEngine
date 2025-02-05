#pragma once
#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace Kong
{
    class KongRenderModule;

    // 返回渲染结果信息
    struct RenderResultInfo
    {
        GLuint frameBuffer {GL_NONE};
        GLuint resultColor {GL_NONE};
        GLuint resultDepth {GL_NONE};
        GLuint resultBloom {GL_NONE};
        GLuint resultPosition {GL_NONE};
    };

    // 渲染传入信息
    struct RenderInputInfo
    {
        GLuint frameBuffer {GL_NONE};
    };

    enum class RenderSystemType : uint8_t
    {
        DEFERRED = 0,
        SKYBOX,
        POST_PROCESS,
        SS_REFLECTION,
        NONE,
    };
    
    // 渲染系统，每个渲染效果或者阶段都独立出来
    class KongRenderSystem
    {
    public:
        KongRenderSystem() = default;
        virtual ~KongRenderSystem() = default;

        // 禁止system之间的复制操作
        KongRenderSystem(const KongRenderSystem&) = delete;
        KongRenderSystem& operator=(const KongRenderSystem&) = delete;
        
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
