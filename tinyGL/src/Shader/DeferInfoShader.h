#pragma once
#include "Shader.h"

namespace Kong
{
    class DeferInfoShader : public Shader
    {
    public:
        DeferInfoShader() = default;
        virtual void InitDefaultShader() override;
        void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
 
    };
    
    class DeferredBRDFShader : public Shader
    {
    public:
        DeferredBRDFShader() = default;
        void InitDefaultShader() override;
        void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
    };
}
