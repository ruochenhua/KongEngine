#pragma once
#include "Shader.h"

namespace Kong
{
    class ShadowMapShader : public Shader
    {
    public:
        ShadowMapShader() = default;
    };

    class DirectionalLightShadowMapShader : public ShadowMapShader
    {
    public:
        DirectionalLightShadowMapShader();
    };

    class DirectionalLightCSMShader : public ShadowMapShader
    {
    public:
        DirectionalLightCSMShader();
    };

    class PointLightShadowMapShader :public ShadowMapShader
    {
    public:
        PointLightShadowMapShader();
    };
}
