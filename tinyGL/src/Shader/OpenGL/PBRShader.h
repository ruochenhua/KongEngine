#pragma once
#include "OpenGLShader.h"

namespace Kong
{
    // todo：或许应该改名为pbrshader
    class PBRShader : public OpenGLShader
    {
    public:
        PBRShader();
        
        void UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material) override;
    };
}