#pragma once
#include "Shader.h"

namespace Kong
{
    // 颜色混合类型shader
    class BlendShader : public Shader
    {
    public:
        BlendShader() = default;
        
        void UpdateRenderData(const SRenderInfo& render_info, const SSceneRenderInfo& scene_render_info) override;
        virtual void InitDefaultShader() override;
    };
}

