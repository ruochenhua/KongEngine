#include "PostprocessShader.h"

#include "render.h"
using namespace Kong;

vector<GLuint> FinalPostprocessShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{
    Use();
    glBindVertexArray(screen_quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_list[0]);  // 原本场景的贴图
    if(texture_list.size() > 1)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture_list[1]);  // 泛光blur的贴图
        SetBool("bloom", true);
    }
    else
    {
        SetBool("bloom", false);
    }
    auto main_cam = CRender::GetRender()->GetCamera();
    if(main_cam)
    {
        SetFloat("exposure", main_cam->exposure);    
    }
    
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindVertexArray(GL_NONE);

    return vector<GLuint>();
}

void FinalPostprocessShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
    
    Use();
    SetInt("scene_texture", 0);
    SetInt("bright_texture", 1);
}

void FinalPostprocessShader::InitPostProcessShader(unsigned width, unsigned height)
{
    InitDefaultShader();
    GenerateTexture(width, height);
}

void FinalPostprocessShader::GenerateTexture(unsigned width, unsigned height)
{
}

void GaussianBlurShader::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
    Shader::UpdateRenderData(mesh, scene_render_info);
}

vector<GLuint> GaussianBlurShader::Draw(const vector<GLuint>& texture_list, GLuint screen_quad_vao)
{
    bool horizontal = true, first_iteration = true;
    Use();
    for (unsigned i = 0; i < blur_amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo[horizontal]);
        //SetInt("horizontal", horizontal);
        glUniform1i(glGetUniformLocation(shader_id, "horizontal"), horizontal);
        glActiveTexture(GL_TEXTURE0);
        // bind texture of other framebuffer (or scene if first iteration) screen_quad_texture[1]
        glBindTexture(GL_TEXTURE_2D, first_iteration ? texture_list[0] : blur_texture[!horizontal]);
        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(GL_NONE);
        
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //blur_texture[!horizontal]; // 这个是输出的贴图
    return {blur_texture[!horizontal]};
}

void GaussianBlurShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess/gaussian_blur.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess/gaussian_blur.frag")}
    };
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
    glUseProgram(shader_id);
    SetInt("bright_texture", 0);
    
    glGenFramebuffers(2, blur_fbo);
}

void GaussianBlurShader::InitPostProcessShader(unsigned width, unsigned height)
{
    InitDefaultShader();
    GenerateTexture(width, height);
}

void GaussianBlurShader::GenerateTexture(unsigned width, unsigned height)
{
    if(!blur_fbo[0])
    {
        glGenFramebuffers(2, blur_fbo);
    }

    // 原来有贴图就删掉
    if(blur_texture[0])
    {
       glDeleteTextures(2, blur_texture);
    }
    
    glGenTextures(2, blur_texture);
    for(unsigned i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo[i]);

        glBindTexture(GL_TEXTURE_2D, blur_texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
        0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_texture[i], 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
