#pragma once
#include "Shader.h"

namespace tinyGL
{
    // 简单发光shader
    class EmitShader : public Shader
    {
    public:
        EmitShader() = default;
        void UpdateRenderData(const CMesh& mesh,
            const SSceneRenderInfo& scene_render_info) override;
    
        virtual void InitDefaultShader() override;
    };
}