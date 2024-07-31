#pragma once

#include "renderobj.h"

namespace tinyGL
{
    enum class ELightType
    {
        directional_light = 0,
        point_light,
        spot_light,
    };
    
    // light source
    class Light : public SceneObject
    {
    public:
        Light(ELightType in_type)
            : light_type(in_type)
        {
            
        }
        
        glm::vec3 light_color = glm::vec3(0);
        
        ELightType GetLightType() const { return light_type; }
        virtual glm::vec3 GetLightDir() const = 0;
    private:
        ELightType light_type;
    };

    // directional light
    class DirectionalLight : public Light
    {
    public:
        DirectionalLight()
            : Light(ELightType::directional_light)
        {
            
        }

        glm::vec3 GetLightDir() const override;
    };

    // point light
    class PointLight : public Light
    {
    public:
        PointLight();

        glm::vec3 GetLightDir() const override;

    private:
        GLuint depth_cubemap = GL_NONE;
        GLuint depth_map_fbo = GL_NONE;
    };
}
