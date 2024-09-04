#pragma once
#include "Shader.h"

namespace Kong
{
    // 简单发光shader
    class EmitShader : public Shader
    {
    public:
        EmitShader() = default;
        void UpdateRenderData(const SRenderInfo& render_info,
            const SSceneRenderInfo& scene_render_info) override;
    
        virtual void InitDefaultShader() override;
    };
}