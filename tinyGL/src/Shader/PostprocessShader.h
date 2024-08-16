#pragma once
#include "Shader.h"

namespace tinyGL
{
    // 后处理shader
    class PostprocessShader : public Shader
    {
    public:
        PostprocessShader() = default;
        void UpdateRenderData(const CMesh& mesh,
            const SSceneRenderInfo& scene_render_info) override;
        void DrawScreenQuad();

        virtual void InitDefaultShader() override;

        GLuint screen_quad_fbo = GL_NONE;
        bool bloom = false;
    protected:
        // 渲染到屏幕的顶点数据，可以共用        
        GLuint screen_quad_vao = GL_NONE;
        GLuint screen_quad_vbo = GL_NONE;

        // 渲染到屏幕的texture
        // 0: scene texture
        // 1: bright texture(for bloom effect)
        GLuint screen_quad_texture[2] = {GL_NONE, GL_NONE};
        GLuint scene_rbo = GL_NONE;
        
        // bloom effect
        GLuint blur_texture[2] = {GL_NONE, GL_NONE};
        GLuint blur_fbo[2] = {GL_NONE, GL_NONE};
        // todo: 重构后处理流程
        GLuint blur_shader_id = GL_NONE;
    private:
        int window_width = 1024;
        int window_height = 768;

        void GenerateScreenTexture();
    };
}