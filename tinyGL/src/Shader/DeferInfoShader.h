#pragma once
#include "Shader.h"

namespace Kong
{
    class DeferInfoShader : public Shader
    {
    public:
        DeferInfoShader() = default;
        virtual void InitDefaultShader() override;
    };
}
