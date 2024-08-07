#pragma once
#include "Shader.h"

namespace tinyGL
{
    class ShadowMapShader : public Shader
    {
    public:
        ShadowMapShader() = default;

        virtual void InitDefaultShader() {};

        virtual void UpdateShadowMapRender(const glm::vec3&, const glm::mat4&) {};
        
        GLuint shadowmap_texture = GL_NONE;
        GLuint shadowmap_fbo = GL_NONE;
    protected:
        GLfloat near_plane = 1.0f;
        GLfloat far_plane = 30.f;
    };

    class DirectionalLightShadowMapShader : public ShadowMapShader
    {
    public:
        virtual void InitDefaultShader() override;
        virtual void UpdateShadowMapRender(const glm::vec3& light_direction,
            const glm::mat4& model_mat) override;
    };

    class PointLightShadowMapShader :public ShadowMapShader
    {
    public:
        virtual void InitDefaultShader() override;
        virtual void UpdateShadowMapRender(const glm::vec3& light_location,
            const glm::mat4& model_mat) override;
    };
}
