#pragma once
#include "Shader/PostprocessShader.h"
#include "Shader/Shader.h"

namespace Kong
{
    class PostProcess
    {
    public:
        void Init();
        GLuint GetScreenFrameBuffer() const {return screen_quad_fbo;}
        void OnWindowResize(unsigned width, unsigned height);
        
        void Draw();
        
        bool bloom = false;
        int bloom_range = 10;
        
        GLuint screen_quad_texture[2] = {GL_NONE, GL_NONE};
    protected:
        // 渲染到屏幕的顶点数据，可以共用        
        GLuint screen_quad_vao = GL_NONE;
        GLuint screen_quad_vbo = GL_NONE;

        // 渲染到屏幕的frame buffer
        GLuint screen_quad_fbo = GL_NONE;
        
    private:
        void InitQuad();
        void InitScreenTexture();
        // 后处理最后阶段
        shared_ptr<FinalPostprocessShader> final_postprocess;
        // 高斯模糊阶段
        shared_ptr<GaussianBlurShader> gaussian_blur;
        // 渲染到屏幕的texture
        // 0: scene texture
        // 1: bright texture(for bloom effect)
        //GLuint screen_quad_texture[2] = {GL_NONE, GL_NONE};
        GLuint scene_rbo = GL_NONE;
        
        int window_width = 1024;
        int window_height = 768;
    };
}
