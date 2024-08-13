#pragma once
#include "Shader.h"

namespace tinyGL
{
    // 后处理shader
    class PostprocessShader : public Shader
    {
    public:
        PostprocessShader() = default;
        void SetupData(CMesh& mesh) override;
        void UpdateRenderData(const CMesh& mesh,
            const SSceneRenderInfo& scene_render_info) override;
        void DrawScreenQuad();

        virtual void InitDefaultShader() override;

        GLuint screen_quad_fbo = GL_NONE;
    protected:
        // 渲染到屏幕的texture
        GLuint screen_quad_texture = GL_NONE;
        GLuint scene_rbo = GL_NONE;
        GLuint screen_quad_vao = GL_NONE;
        GLuint screen_quad_vbo = GL_NONE;

    private:
        int window_width = 1024;
        int window_height = 768;

        void GenerateScreenTexture();
    };
}