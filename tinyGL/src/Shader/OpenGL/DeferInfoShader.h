#pragma once
#include "OpenGLShader.h"

namespace Kong
{
    class DeferInfoShader : public OpenGLShader
    {
    public:
        DeferInfoShader();
        void UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material) override;
 
    };
    
    class DeferredBRDFShader : public OpenGLShader
    {
    public:
        DeferredBRDFShader();
        void UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material) override;
    };

    class DeferredTerrainInfoShader : public OpenGLShader
    {
    public:
        DeferredTerrainInfoShader();
    };
    
    class SSAOShader : public OpenGLShader
    {
    public:
        SSAOShader();
    };

    class SSReflectionShader : public OpenGLShader
    {
    public:
        SSReflectionShader();
    };
}
