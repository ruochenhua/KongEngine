#pragma once
#include "assimp/code/AssetLib/glTF2/glTF2Exporter.h"
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
        void RenderUI();
        void SetPositionTexture(GLuint texture) {position_texture = texture;}
        
        bool enable_bloom = false;
        // 开启和关闭景深
        bool enable_DOF = false;
        int bloom_range = 10;

        float focus_distance = 5.f; //景深focus的位置
        glm::vec2 focus_threshold = glm::vec2(1.0, 15.0);
        int dilate_size = 3;
        float dilate_separation = 0.5;
        
        // 0: 正常场景；1：bloom颜色；
        GLuint screen_quad_texture[2] = {GL_NONE, GL_NONE};
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
        // 后处理最后阶段
        shared_ptr<FinalPostprocessShader> final_postprocess;
        // 高斯模糊阶段
        shared_ptr<GaussianBlurShader> gaussian_blur;
        // 膨胀模糊效果
        shared_ptr<DilatePostprocessShader> dilate_blur;
        // 景深效果
        shared_ptr<DOFPostprocessShader> dof_process;
        
        // 渲染到屏幕的texture
        // 0: scene texture
        // 1: bright texture(for bloom effect)
        //GLuint screen_quad_texture[2] = {GL_NONE, GL_NONE};
        GLuint scene_rbo = GL_NONE;
        
        int window_width = 1024;
        int window_height = 768;
    };
}
