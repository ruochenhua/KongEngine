#include "light.h"

using namespace tinyGL;
using namespace glm;

glm::vec3 DirectionalLight::GetLightDir() const
{
    // rotation to direction
    /*
    * x = cos(yaw)*cos(pitch)
    * y = sin(yaw)*cos(pitch)
    * z = sin(pitch)
     */
    vec3 dir;
    dir.x = cos(rotation.z) * cos(rotation.y);
    dir.y = sin(rotation.z) * cos(rotation.y);
    dir.z = sin(rotation.y);
    return normalize(dir);
}

vec3 PointLight::GetLightDir() const
{
    // point light has no direction
    return vec3(0);
}
