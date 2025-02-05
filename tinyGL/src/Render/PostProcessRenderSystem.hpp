#pragma once
#include "RenderSystem.hpp"
#include "Shader/PostprocessShader.h"
#include "Shader/Shader.h"


namespace Kong
{
    class PostProcessRenderSystem : public KongRenderSystem
    {
    public:
        PostProcessRenderSystem();
        
        void Init() override;
        void OnWindowResize(unsigned width, unsigned height);
        
        RenderResultInfo Draw(double delta, const RenderResultInfo& render_result_info,
            KongRenderModule* render_module) override;
        
        void DrawUI() override;
        
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
    
    protected:
        // 渲染到屏幕的顶点数据，可以共用        
        GLuint screen_quad_vao = GL_NONE;
        GLuint screen_quad_vbo = GL_NONE;
        
    private:
        void InitQuad();

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
        
        
        
        int window_width = 1024;
        int window_height = 768;
    };
}
