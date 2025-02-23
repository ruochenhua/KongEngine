#pragma once
#include "OpenGLShader.h"

namespace Kong
{
    // 颜色混合类型shader
    class BlendShader : public OpenGLShader
    {
    public:
        BlendShader();
        
        void UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material) override;
    };
}

