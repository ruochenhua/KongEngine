#pragma once
#include "Shader.h"

namespace Kong
{
    // 天空盒渲染
    class SkyboxShader : public Shader
    {
    public:
        SkyboxShader();
    };

    class AtmosphereShader : public Shader
    {
    public:
        AtmosphereShader();
    };
    
    // 等距柱状投影图到立方体贴图转换计算
    class EquirectangularToCubemapShader : public Shader
    {
    public:
        EquirectangularToCubemapShader();
    };

    // 辐照度贴图预计算
    class IrradianceCalculationShader : public  Shader
    {
    public:
        IrradianceCalculationShader();
    };

    // 预滤波贴图预计算
    class PrefilterCalculationShader : public  Shader
    {
    public:
        PrefilterCalculationShader();
    };
    
    // brdf参数查找表贴图预计算
    class BRDFLutCalculationShader :public Shader
    {
    public:
        BRDFLutCalculationShader();
    };
}
