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

    // dilate blur, 用于景深
    class DilatePostprocessShader : public PostprocessShader
    {
    public:
        DilatePostprocessShader() = default;
        virtual void InitDefaultShader() override;
        virtual void InitPostProcessShader(unsigned width, unsigned height) override;
        void SetBlurAmount(unsigned amount) {blur_amount = amount;}
        virtual void GenerateTexture(unsigned width, unsigned height) override;
    
    protected:
        unsigned blur_amount = 10;
        GLuint blur_texture = 0;
        GLuint blur_fbo = 0;
    };
    
}