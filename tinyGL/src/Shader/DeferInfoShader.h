#pragma once
#include "Shader.h"

namespace Kong
{
    class DeferInfoShader : public Shader
    {
    public:
        DeferInfoShader() = default;
        virtual void InitDefaultShader() override;
        void UpdateRenderData(const SMaterial& render_material, const SSceneRenderInfo& scene_render_info) override;
 
    };
    
    class DeferredBRDFShader : public Shader
    {
    public:
        DeferredBRDFShader() = default;
        void InitDefaultShader() override;
        void UpdateRenderData(const SMaterial& render_material, const SSceneRenderInfo& scene_render_info) override;
    };

    class SSAOShader : public Shader
    {
    public:
        SSAOShader() = default;
        virtual void InitDefaultShader() override;
        void UpdateRenderData(const SMaterial& render_material, const SSceneRenderInfo& scene_render_info);
    };
}
