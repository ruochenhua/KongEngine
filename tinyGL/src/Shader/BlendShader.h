#pragma once
#include "Shader.h"

namespace Kong
{
    // 颜色混合类型shader
    class BlendShader : public Shader
    {
    public:
        BlendShader();
        
        void UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info) override;
    };
}

