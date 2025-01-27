#pragma once
#include "Shader.h"

namespace Kong
{
    class DeferInfoShader : public Shader
    {
    public:
        DeferInfoShader();
        void UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info) override;
 
    };
    
    class DeferredBRDFShader : public Shader
    {
    public:
        DeferredBRDFShader();
        void UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info) override;
    };

    class DeferredTerrainInfoShader : public Shader
    {
    public:
        DeferredTerrainInfoShader();
    };
    
    class SSAOShader : public Shader
    {
    public:
        SSAOShader();
    };

    class SSReflectionShader : public Shader
    {
    public:
        SSReflectionShader();
    };
}
