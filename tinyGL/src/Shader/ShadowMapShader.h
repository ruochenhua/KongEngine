#pragma once
#include "Shader.h"

namespace Kong
{
    class ShadowMapShader : public Shader
    {
    public:
        ShadowMapShader() = default;

        virtual void InitDefaultShader() {};

        virtual void UpdateShadowMapRender(const glm::vec3& light_dir,
            const glm::mat4& model_mat,
            const glm::vec2& near_far_plane)
        {};
        
    };

    class DirectionalLightShadowMapShader : public ShadowMapShader
    {
    public:
        virtual void InitDefaultShader() override;
    };

    class PointLightShadowMapShader :public ShadowMapShader
    {
    public:
        virtual void InitDefaultShader() override;
        virtual void UpdateShadowMapRender(const glm::vec3& light_location,
        const glm::mat4& model_mat,
        const glm::vec2& near_far_plane) override;
    };
}
