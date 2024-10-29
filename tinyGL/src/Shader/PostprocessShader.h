#pragma once
#include "Shader.h"

namespace Kong
{
    // 后处理shader
    class PostprocessShader : public Shader
    {
    public:
        PostprocessShader() = default;
        virtual vector<GLuint> Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) = 0;

        virtual void InitDefaultShader() override = 0;
        
        virtual void InitPostProcessShader(unsigned width, unsigned height) = 0;
        virtual void GenerateTexture(unsigned width, unsigned height) = 0;
    };

    // 预后处理shader，将一些数据在这里整合
    class PrePostProcessShader : public PostprocessShader
    {
    public:
        PrePostProcessShader() = default;
        virtual vector<GLuint> Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;
        virtual void InitDefaultShader() override;
        virtual void GenerateTexture(unsigned width, unsigned height) override;
        virtual void InitPostProcessShader(unsigned width, unsigned height) override;
        
    protected:
        GLuint result_fbo = 0;
        GLuint result_texture = 0;
    };
    
    class FinalPostprocessShader : public PostprocessShader
    {
    public:
        FinalPostprocessShader() = default;
        vector<GLuint> Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;

        virtual void InitDefaultShader() override;
        virtual void InitPostProcessShader(unsigned width, unsigned height) override;
        virtual void GenerateTexture(unsigned width, unsigned height) override;
    };

    class GaussianBlurShader : public PostprocessShader
    {
    public:
        GaussianBlurShader() = default;
        vector<GLuint> Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;

        virtual void InitDefaultShader() override;
        virtual void InitPostProcessShader(unsigned width, unsigned height) override;
        void SetBlurAmount(unsigned amount) {blur_amount = amount;}
        virtual void GenerateTexture(unsigned width, unsigned height) override;
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
        virtual void InitDefaultShader() override;
        virtual void InitPostProcessShader(unsigned width, unsigned height) override;
        
        virtual void GenerateTexture(unsigned width, unsigned height) override;

        vector<GLuint> Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;
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
        virtual void InitDefaultShader() override;
        virtual void InitPostProcessShader(unsigned width, unsigned height) override;
        void SetFocusDistance(float distance, const glm::vec2& threshold);

        // todo generate texture 可以合并一下 
        virtual void GenerateTexture(unsigned width, unsigned height) override;
        vector<GLuint> Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao) override;
        
        
    protected:
        GLuint DOF_texture = 0;
        GLuint DOF_fbo = 0;
        float focus_distance = 3.0f;
        glm::vec2 focus_threshold = glm::vec2(1.0, 5.0); 
    };
    
}