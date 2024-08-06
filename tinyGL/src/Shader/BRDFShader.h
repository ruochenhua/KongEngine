#pragma once
#include "Shader.h"

namespace tinyGL
{
    class BRDFShader : public Shader
    {
    public:
        BRDFShader();

        void SetupData(CMesh& mesh) override;
        void UpdateRenderData(const CMesh& mesh,
            const glm::mat4& actor_model_mat,
            const SSceneRenderInfo& scene_render_info) override;
    };
}