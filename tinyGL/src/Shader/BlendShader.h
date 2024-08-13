#pragma once
#include "Shader.h"

namespace tinyGL
{
    // 颜色混合类型shader
    class BlendShader : public Shader
    {
    public:
        BlendShader() = default;
        
        void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
        virtual void InitDefaultShader() override;
    };
}

