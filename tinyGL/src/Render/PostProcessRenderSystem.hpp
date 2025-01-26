#pragma once
#include "Shader/PostprocessShader.h"
#include "Shader/Shader.h"

static constexpr unsigned PP_TEXTURE_COUNT = 3;

namespace Kong
{
    class PostProcessRenderSystem
    {
    public:
        void Init();
        GLuint GetScreenFrameBuffer() const {return screen_quad_fbo;}
        GLuint GetScreenTexture() const {return screen_quad_texture[0];}
        void OnWindowResize(unsigned width, unsigned height);
        
        void Draw();
        void RenderUI();
        void SetPositionTexture(GLuint texture) {position_texture = texture;}
        
        bool enable_bloom = false;
        // 开启和关闭景深
        bool enable_DOF = false;
        // 开启和关闭god_ray
        bool enable_god_ray = false;
        
        int bloom_range = 10;

        float focus_distance = 5.f; //景深focus的位置
        glm::vec2 focus_threshold = glm::vec2(1.0, 15.0);
        int dilate_size = 3;
        float dilate_separation = 0.5;
        
        // 0: 正常场景；1：bloom颜色；2：反射颜色
        GLuint screen_quad_texture[PP_TEXTURE_COUNT] = {GL_NONE};
    protected:
        // 渲染到屏幕的顶点数据，可以共用        
        GLuint screen_quad_vao = GL_NONE;
        GLuint screen_quad_vbo = GL_NONE;

        // 渲染到屏幕的frame buffer
        GLuint screen_quad_fbo = GL_NONE;

        // 位置贴图，从延迟渲染的部分得到
        GLuint position_texture = GL_NONE;
        
    private:
        void InitQuad();
        void InitScreenTexture();

        // 预后处理阶段
        shared_ptr<CombineProcessShader> combine_process;
        // 高斯模糊阶段
        shared_ptr<GaussianBlurShader> gaussian_blur;
        // 膨胀模糊效果
        shared_ptr<DilatePostprocessShader> dilate_blur;
        // 景深效果
        shared_ptr<DOFPostprocessShader> dof_process;
        // 径向模糊效果
        shared_ptr<RadicalBlurShader> radical_blur;
        
        // 后处理最后阶段
        shared_ptr<FinalPostprocessShader> final_postprocess;
        
        // 渲染到屏幕的texture
        // 0: scene texture
        // 1: bright texture(for bloom effect)
        //GLuint screen_quad_texture[2] = {GL_NONE, GL_NONE};
        GLuint scene_rbo = GL_NONE;
        
        int window_width = 1024;
        int window_height = 768;
    };
}
