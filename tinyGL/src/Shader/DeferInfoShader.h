#pragma once
#include "Shader.h"

namespace Kong
{
    class DeferInfoShader : public Shader
    {
    public:
        DeferInfoShader();
        void UpdateRenderData(const SMaterialInfo& render_material) override;
 
    };
    
    class DeferredBRDFShader : public Shader
    {
    public:
        DeferredBRDFShader();
        void UpdateRenderData(const SMaterialInfo& render_material) override;
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
