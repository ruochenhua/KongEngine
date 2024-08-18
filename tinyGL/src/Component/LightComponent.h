#pragma once

#include "Component.h"
#include "Component/Mesh/MeshComponent.h"

namespace tinyGL
{
    class ShadowMapShader;
    enum class ELightType
    {
        directional_light = 0,
        point_light,
        spot_light,
    };
    
    // light source
    class CLightComponent : public CComponent
    {
    public:
        CLightComponent(ELightType in_type);
        
        glm::vec3 light_color = glm::vec3(0);
        
        ELightType GetLightType() const { return light_type; }
        virtual glm::vec3 GetLightDir() const = 0;
        
        virtual void RenderShadowMap() = 0;
        GLuint GetShadowMapTexture() const;
    protected:
        shared_ptr<ShadowMapShader> shadowmap_shader;
    private:
        ELightType light_type;
    };

    // directional light
    class CDirectionalLightComponent : public CLightComponent
    {
    public:
        CDirectionalLightComponent();

        glm::vec3 GetLightDir() const override;
        void RenderShadowMap() override;
        void SetLightDir(const glm::vec3& rotation);
        glm::mat4 light_space_mat;
    private:
        glm::vec3 light_dir;
    };

    // point light
    class CPointLightComponent : public CLightComponent
    {
    public:
        CPointLightComponent();

        glm::vec3 GetLightDir() const override;
        void RenderShadowMap() override;
        glm::vec3 GetLightLocation() const;
        void SetLightLocation(const glm::vec3& in_light_location);
    private:
        glm::vec3 light_location;
    };
}
