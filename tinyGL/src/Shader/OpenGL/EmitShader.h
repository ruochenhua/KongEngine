#pragma once
#include "OpenGLShader.h"

namespace Kong
{
    // 简单发光shader
    class EmitShader : public OpenGLShader
    {
    public:
        EmitShader();
        void UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material) override;
    };
}