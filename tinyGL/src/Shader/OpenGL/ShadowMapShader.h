#pragma once
#include "OpenGLShader.h"

namespace Kong
{
    class ShadowMapShader : public OpenGLShader
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
