#pragma once
#include "Shader.h"

namespace Kong
{
    // 颜色混合类型shader
    class BlendShader : public Shader
    {
    public:
        BlendShader();
        
        void UpdateRenderData(const SMaterialInfo& render_material) override;
    };
}

