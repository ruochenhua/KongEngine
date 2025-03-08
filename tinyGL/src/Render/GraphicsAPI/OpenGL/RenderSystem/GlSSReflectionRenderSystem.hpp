#pragma once
#include <memory>

#include "OpenGLRenderSystem.hpp"
#include "Component/Mesh/QuadShape.h"

namespace Kong
{
    class SSReflectionShader;

    class GlSSReflectionRenderSystem : public OpenGLRenderSystem
    {
    public:
        GlSSReflectionRenderSystem();
        
        void Init() override;
        RenderResultInfo Draw(double delta,
            const RenderResultInfo& render_result_info,
            KongRenderModule* render_module) override;
        void DrawUI() override;

    private:
        
        // 屏幕空间反射
        std::shared_ptr<SSReflectionShader> m_ssReflectionShader;
    };
}
