#include "PostprocessShader.h"

#include "Render/RenderModule.hpp"
#include "Scene.hpp"
using namespace Kong;

CombineProcessShader::CombineProcessShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess/pre_postprocess.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess/pre_postprocess.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
    
    SetInt("scene_texture", 0);
    SetInt("added_texture", 1);
}

GLuint CombineProcessShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{

    Use();
    glBindFramebuffer(GL_FRAMEBUFFER, result_fbo);
    glBindVertexArray(screen_quad_vao);

    SetInt("combine_mode", combine_mode);
    for(int i = 0; i < texture_list.size(); i++)
    {
#if USE_DSA
        glBindTextureUnit(i, texture_list[i]);
#else
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture_list[i]);
#endif
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindVertexArray(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return result_texture;
}

void CombineProcessShader::GenerateTexture(unsigned width, unsigned height)
{
    if(!result_fbo)
    {
        glGenFramebuffers(1, &result_fbo);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, result_fbo);
    TextureBuilder::CreateTexture2D(result_texture, width, height, GL_RGBA);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CombineProcessShader::InitPostProcessShader(unsigned width, unsigned height)
{
    GenerateTexture(width, height);
}

FinalPostprocessShader::FinalPostprocessShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
    
    SetInt("scene_texture", 0);
    SetInt("bright_texture", 1);
}

GLuint FinalPostprocessShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{
    Use();
    glBindVertexArray(screen_quad_vao);
#if USE_DSA
    glBindTextureUnit(0, texture_list[0]);
#else
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_list[0]);  // 原本场景的贴图
#endif
    if(texture_list.size() > 1)
    {
#if USE_DSA
        glBindTextureUnit(1, texture_list[1]);
#else
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture_list[1]);  // 泛光blur的贴图
#endif
        SetBool("bloom", true);
    }
    else
    {
        SetBool("bloom", false);
    }
    
    auto main_cam = KongRenderModule::GetRenderModule().GetCamera();
    if(main_cam)
    {
        SetFloat("exposure", main_cam->exposure);    
    }
    
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindVertexArray(GL_NONE);

    return 0;
}

void FinalPostprocessShader::InitPostProcessShader(unsigned width, unsigned height)
{
    GenerateTexture(width, height);
}

void FinalPostprocessShader::GenerateTexture(unsigned width, unsigned height)
{
}

GaussianBlurShader::GaussianBlurShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess/gaussian_blur.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess/gaussian_blur.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
    
    SetInt("bright_texture", 0);
}

GLuint GaussianBlurShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{
    bool horizontal = true, first_iteration = true;
    Use();
    for (unsigned i = 0; i < blur_amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo[horizontal]);
        //SetInt("horizontal", horizontal);
        glUniform1i(glGetUniformLocation(shader_id, "horizontal"), horizontal);
#if USE_DSA
        glBindTextureUnit(0, first_iteration ? texture_list[0] : blur_texture[!horizontal]);
#else
        glActiveTexture(GL_TEXTURE0);
        // bind texture of other framebuffer (or scene if first iteration) screen_quad_texture[1]
        glBindTexture(GL_TEXTURE_2D, first_iteration ? texture_list[0] : blur_texture[!horizontal]);
#endif
        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(GL_NONE);
        
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //blur_texture[!horizontal]; // 这个是输出的贴图
    return blur_texture[!horizontal];
}


void GaussianBlurShader::InitPostProcessShader(unsigned width, unsigned height)
{
    GenerateTexture(width, height);
}

void GaussianBlurShader::GenerateTexture(unsigned width, unsigned height)
{
    if(!blur_fbo[0])
    {
        glGenFramebuffers(2, blur_fbo);
    }

    for (int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo[i]);
        TextureBuilder::CreateTexture2D(blur_texture[i], width, height, GL_RGBA);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_texture[i], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DilatePostprocessShader::DilatePostprocessShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess/dilate_blur.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess/dilate_blur.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");

    SetInt("scene_texture", 0);
}

void DilatePostprocessShader::InitPostProcessShader(unsigned width, unsigned height)
{
    GenerateTexture(width, height);
}

void DilatePostprocessShader::GenerateTexture(unsigned width, unsigned height)
{
    if(blur_fbo == 0)
    {
        glGenFramebuffers(1, &blur_fbo);
    }

    // 原来有贴图就删掉
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
    TextureBuilder::CreateTexture2D(blur_texture, width, height, GL_RGBA);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

GLuint DilatePostprocessShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{
    Use();
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
    SetInt("size", dilate_size);
    SetFloat("separation", dilate_separation);
#if USE_DSA
    glBindTextureUnit(0, texture_list[0]);
#else
    glActiveTexture(GL_TEXTURE0);
    // bind texture of other framebuffer (or scene if first iteration) screen_quad_texture[1]
    glBindTexture(GL_TEXTURE_2D, texture_list[0]);
#endif
    glBindVertexArray(screen_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 这个是输出的贴图
    return blur_texture;
}

void DilatePostprocessShader::SetParam(int d_size, float d_separation)
{
    dilate_size = d_size;
    dilate_separation = d_separation;
}


DOFPostprocessShader::DOFPostprocessShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess/DOF.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess/DOF.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");

    SetInt("scene_texture", 0);
    SetInt("dilate_texture", 1);
    SetInt("position_texture", 2);
}

void DOFPostprocessShader::InitPostProcessShader(unsigned width, unsigned height)
{
    GenerateTexture(width, height);
}

void DOFPostprocessShader::SetFocusDistance(float distance, const vec2& threshold)
{
    focus_distance = distance;
    focus_threshold = threshold;
}

void DOFPostprocessShader::GenerateTexture(unsigned width, unsigned height)
{
    if(DOF_fbo == 0)
    {
        glGenFramebuffers(1, &DOF_fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, DOF_fbo);
    TextureBuilder::CreateTexture2D(DOF_texture, width, height, GL_RGBA);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DOF_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint DOFPostprocessShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{
    Use();
    glBindFramebuffer(GL_FRAMEBUFFER, DOF_fbo);
    SetFloat("focus_distance", focus_distance);
    SetVec2("focus_threshold", focus_threshold);
    
    assert(texture_list.size() == 3);
#if USE_DSA
    glBindTextureUnit(0, texture_list[0]);
    glBindTextureUnit(1, texture_list[1]);
    glBindTextureUnit(2, texture_list[2]);
#else
    // 场景渲染贴图
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_list[0]);
    // dilate模糊贴图
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_list[1]);
    // 位置贴图
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_list[2]);
#endif
    
    glBindVertexArray(screen_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 这个是输出的贴图
    return DOF_texture;
}

RadicalBlurShader::RadicalBlurShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess/radical_blur.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess/radical_blur.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
    
    SetInt("bright_texture", 0);
}

void RadicalBlurShader::InitPostProcessShader(unsigned width, unsigned height)
{
    GenerateTexture(width, height);
}

void RadicalBlurShader::GenerateTexture(unsigned width, unsigned height)
{
    if(!blur_fbo)
    {
        glGenFramebuffers(1, &blur_fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
    TextureBuilder::CreateTexture2D(blur_texture, width, height, GL_RGBA);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint RadicalBlurShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{
    Use();
    SetFloat("blur_amount", blur_amount);
    
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
#if USE_DSA
    glBindTextureUnit(0, texture_list[0]);
#else
    glActiveTexture(GL_TEXTURE0);
    // bind texture of other framebuffer (or scene if first iteration) screen_quad_texture[1]
    glBindTexture(GL_TEXTURE_2D, texture_list[0]);
#endif
    glBindVertexArray(screen_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 这个是输出的贴图
    return blur_texture;
}
