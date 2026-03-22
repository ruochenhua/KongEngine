#pragma once
#include <memory>

#include "Render/Abstraction/RHIRenderSubsystems.hpp"
#include "OpenGLRenderSystem.hpp"
#include "Component/Mesh/QuadShape.h"

namespace Kong
{
    class SSReflectionShader;

    class GlSSReflectionRenderSystem : public OpenGLRenderSystem, public IRHIRenderSubsystem
    {
    public:
        RHISubsystemKind GetRHISubsystemKind() const noexcept override
        {
            return RHISubsystemKind::ScreenSpaceReflection;
        }

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
