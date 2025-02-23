#pragma once
#include "OpenGLShader.h"

namespace Kong
{
    // 天空盒渲染
    class SkyboxShader : public OpenGLShader
    {
    public:
        SkyboxShader();
    };

    struct AtmosphereStorageData
    {
        float coverage = 0.15f, cloud_speed = 200.f, crispiness = 50.f, curliness = 0.5f, density = 0.02f, absorption = 0.35f;
        bool enable_powder = false;
        
        float earth_radius = 600000.0, sphere_inner_radius = 3500.0f, sphere_outer_radius = 12000.0f;

        glm::vec3 cloud_color_top = glm::vec3(169.f, 149.f, 149.f) * (1.5f/255.f);
        glm::vec3 cloud_color_bottom = glm::vec3(65.f, 70.f, 80.f) * (1.5f/255.f);

        glm::vec3 sky_color_top = glm::vec3(0.5, 0.7, 0.8)*1.05f;
        glm::vec3 sky_color_bottom = glm::vec3(0.9, 0.9, 0.95);

        glm::mat4 inv_view;
        glm::mat4 inv_proj;
        glm::vec2 iResolution;
    };
    
    class AtmosphereShader : public OpenGLShader
    {
    public:
        AtmosphereShader();
    };
    
    // 等距柱状投影图到立方体贴图转换计算
    class EquirectangularToCubemapShader : public OpenGLShader
    {
    public:
        EquirectangularToCubemapShader();
    };

    // 辐照度贴图预计算
    class IrradianceCalculationShader : public  OpenGLShader
    {
    public:
        IrradianceCalculationShader();
    };

    // 预滤波贴图预计算
    class PrefilterCalculationShader : public  OpenGLShader
    {
    public:
        PrefilterCalculationShader();
    };
    
    // brdf参数查找表贴图预计算
    class BRDFLutCalculationShader :public OpenGLShader
    {
    public:
        BRDFLutCalculationShader();
    };
}
