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
        Light(ELightType in_type);

        
        glm::vec3 light_color = glm::vec3(0);
        
        ELightType GetLightType() const { return light_type; }
        virtual glm::vec3 GetLightDir() const = 0;
        
        
        GLuint shadowmap_texture;
        
        glm::mat4 light_space_mat;
        
        virtual void RenderShadowMap() = 0;
    protected:
        GLfloat near_plane;
        GLfloat far_plane;
        GLuint shadowmap_fbo;
        GLuint shadowmap_shader_id;
    private:
        ELightType light_type;
    };

    // directional light
    class DirectionalLight : public Light
    {
    public:
        DirectionalLight();

        glm::vec3 GetLightDir() const override;
        void RenderShadowMap() override;
    };

    // point light
    class PointLight : public Light
    {
    public:
        PointLight();

        glm::vec3 GetLightDir() const override;
        void RenderShadowMap() override;
    
    };
}
