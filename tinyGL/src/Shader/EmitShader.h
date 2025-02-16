#pragma once
#include "Shader.h"

namespace Kong
{
    // 简单发光shader
    class EmitShader : public Shader
    {
    public:
        EmitShader();
        void UpdateRenderData(const SMaterialInfo& render_material) override;
    };
}