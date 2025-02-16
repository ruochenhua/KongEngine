#pragma once
#include "Shader.h"

namespace Kong
{
    // todo：或许应该改名为pbrshader
    class PBRShader : public Shader
    {
    public:
        PBRShader();
        
        void UpdateRenderData(const SMaterial& render_material) override;
    };
}