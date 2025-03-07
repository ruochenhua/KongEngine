#pragma once

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
    };

    // 渲染传入信息
    struct RenderInputInfo
    {
        GLuint frameBuffer {GL_NONE};
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
        
        virtual RenderResultInfo Draw(double delta, const RenderResultInfo& render_result_info,
            KongRenderModule* render_module) = 0;
        
        virtual void Init() = 0;
        
    protected:
        GLuint m_frameBuffer {GL_NONE};
    };
}
