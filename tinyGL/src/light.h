#pragma once

#include "common.h"
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
        {}
        
        glm::vec3 light_color = glm::vec3(0);
        
        ELightType GetLightType() const;
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
    };

    // point light
    class PointLight : public Light
    {
    public:
        PointLight()
            : Light(ELightType::point_light)
        {
            
        }
    };
}
