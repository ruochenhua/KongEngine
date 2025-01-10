#pragma once
#include "Shader.h"

namespace Kong
{
    // 后处理shader
    class PostprocessShader : public Shader
    {
    public:
        PostprocessShader() = default;
        virtual GLuint Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) = 0;

        void InitDefaultShader() override = 0;
        
        virtual void InitPostProcessShader(unsigned width, unsigned height) = 0;
        virtual void GenerateTexture(unsigned width, unsigned height) = 0;
    };

    // 通用处理，主要是结合两次scene渲染
    class CombineProcessShader : public PostprocessShader
    {
    public:
        // 结合的方式
        enum CombineMode
        {
            Add = 0,    // 单纯的叠加    
            Alpha,      // 以第一张贴图为底图，后续的贴图根据本身的alpha值来叠加
        };
        
        CombineProcessShader() = default;
        GLuint Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;
        void InitDefaultShader() override;
        void GenerateTexture(unsigned width, unsigned height) override;
        void InitPostProcessShader(unsigned width, unsigned height) override;
        void SetCombineMode(CombineMode mode) {combine_mode = mode;}
    protected:
        GLuint result_fbo = 0;
        GLuint result_texture = 0;
        CombineMode combine_mode = Add;
        
    };
    
    class FinalPostprocessShader : public PostprocessShader
    {
    public:
        FinalPostprocessShader() = default;
        GLuint Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;

        void InitDefaultShader() override;
        void InitPostProcessShader(unsigned width, unsigned height) override;
        void GenerateTexture(unsigned width, unsigned height) override;
    };

    class GaussianBlurShader : public PostprocessShader
    {
    public:
        GaussianBlurShader() = default;
        GLuint Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;

        void InitDefaultShader() override;
        void InitPostProcessShader(unsigned width, unsigned height) override;
        void SetBlurAmount(unsigned amount) {blur_amount = amount;}
        void GenerateTexture(unsigned width, unsigned height) override;
    protected:
        GLuint blur_texture[2] = {GL_NONE, GL_NONE};
        GLuint blur_fbo[2] = {GL_NONE, GL_NONE};
        
    private:
        unsigned blur_amount = 10;
    };

    // dilate blur
    class DilatePostprocessShader : public PostprocessShader
    {
    public:
        DilatePostprocessShader() = default;
        void InitDefaultShader() override;
        void InitPostProcessShader(unsigned width, unsigned height) override;
        
        void GenerateTexture(unsigned width, unsigned height) override;

        GLuint Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;
        void SetParam(int dilate_size, float dilate_separation);

    protected:
        GLuint blur_texture = 0;
        GLuint blur_fbo = 0;

        int dilate_size = 0;
        float dilate_separation = 1.0f;
    };

    // 景深计算shader
    class DOFPostprocessShader : public PostprocessShader
    {
    public:
        DOFPostprocessShader() = default;
        void InitDefaultShader() override;
        void InitPostProcessShader(unsigned width, unsigned height) override;
        void SetFocusDistance(float distance, const glm::vec2& threshold);

        // todo generate texture 可以合并一下 
        void GenerateTexture(unsigned width, unsigned height) override;
        GLuint Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;
        
        
    protected:
        GLuint DOF_texture = 0;
        GLuint DOF_fbo = 0;
        float focus_distance = 3.0f;
        glm::vec2 focus_threshold = glm::vec2(1.0, 5.0); 
    };

    // 径向模糊
    class RadicalBlurShader : public PostprocessShader
    {
    public:
        RadicalBlurShader() = default;
        void InitDefaultShader() override;
        void InitPostProcessShader(unsigned width, unsigned height) override;

        void SetBlurAmount(float amount) {blur_amount = amount;}
         void GenerateTexture(unsigned width, unsigned height) override;
        GLuint Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;
    private:
        float blur_amount = 1.0f;
        GLuint blur_texture = 0;
        GLuint blur_fbo = 0;
    };
    
}