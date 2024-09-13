#pragma once

#include "Component.h"
#include "Component/Mesh/MeshComponent.h"

namespace Kong
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
        virtual GLuint GetShadowMapTexture() const;
        bool b_make_shadow = false;

        virtual void TurnOnShadowMap(bool b_turn_on) = 0;
    protected:
        shared_ptr<Shader> shadowmap_shader;
        GLuint shadowmap_texture = GL_NONE;
        GLuint shadowmap_fbo = GL_NONE;
        
    private:
        ELightType light_type;
    };

    // directional light
    class CDirectionalLightComponent : public CLightComponent
    {
    public:
        CDirectionalLightComponent();
        virtual GLuint GetShadowMapTexture() const override;
        glm::vec3 GetLightDir() const override;
        void RenderShadowMap() override;
        void SetLightDir(const glm::vec3& rotation);

        void TurnOnShadowMap(bool b_turn_on) override;
        glm::mat4 light_space_mat;
        // 级联阴影
        glm::vec2 camera_near_far;
        vector<float> csm_distances;
        GLuint csm_texture = GL_NONE;
        vector<glm::mat4> light_space_matrices;
    private:
        glm::vec3 light_dir;
        // 计算视锥体范围的AABB在世界坐标下的顶点
        std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& proj_view);
        glm::mat4 CalLightSpaceMatrix(float near, float far);
        std::vector<glm::mat4> GetLightSpaceMatrices();
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
        
        void TurnOnShadowMap(bool b_turn_on) override;
    private:
        void UpdateShadowMapInfo(const glm::mat4& model_mat, const glm::vec2& near_far_plane);
        
        glm::vec3 light_location;
    };
}
