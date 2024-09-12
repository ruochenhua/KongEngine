#pragma once
#include "Shader.h"

namespace Kong
{
    class ShadowMapShader : public Shader
    {
    public:
        ShadowMapShader() = default;

        virtual void InitDefaultShader() = 0;
    };

    class DirectionalLightShadowMapShader : public ShadowMapShader
    {
    public:
        virtual void InitDefaultShader() override;
    };

    class DirectionalLightCSMShader : public ShadowMapShader
    {
    public:
        virtual void InitDefaultShader() override;
    };

    class PointLightShadowMapShader :public ShadowMapShader
    {
    public:
        virtual void InitDefaultShader() override;
    };
}
