#pragma once
#include "Shader.h"

namespace Kong
{
    // 简单发光shader
    class EmitShader : public Shader
    {
    public:
        EmitShader();
        void UpdateRenderData(const SMaterial& render_material,
            const SSceneLightInfo& scene_render_info) override;
    };
}