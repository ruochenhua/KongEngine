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
    // vec3 dir;
    // dir.x = cos(rotation.z) * cos(rotation.y);
    // dir.y = sin(rotation.z) * cos(rotation.y);
    // dir.z = sin(rotation.y);
    // return normalize(dir);
    return normalize(rotation);
}

PointLight::PointLight()
    : Light(ELightType::point_light)
{
    // 创建阴影贴图相关的framebuffer
    // 由于点光源的方向是四面八方的，所以点光源的shadowmap是一个立方体贴图
    glGenFramebuffers(1, &depth_map_fbo);
    // 创建点光源阴影贴图
    glGenTextures(1, &depth_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
    for(GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WITH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


vec3 PointLight::GetLightDir() const
{
    // point light has no direction
    return vec3(0);
}
