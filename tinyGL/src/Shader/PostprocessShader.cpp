#include "PostprocessShader.h"

#include "render.h"
using namespace tinyGL;

void PostprocessShader::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
    //Shader::UpdateRenderData(mesh, actor_model_mat, scene_render_info);
    //glBindVertexArray(scene_vao);
}

void PostprocessShader::DrawScreenQuad()
{
    Use();
    glBindVertexArray(screen_quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_quad_texture);
    auto main_cam = CRender::GetRender()->GetCamera();
    if(main_cam)
    {
        SetFloat("exposure", main_cam->exposure);    
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindVertexArray(GL_NONE);

    Engine engine = Engine::GetEngine();
    int tmp_window_width = engine.GetWindowWidth();
    int tmp_window_height = engine.GetWindowHeight();
    if(tmp_window_width != window_width || window_height != tmp_window_height)
    {
        window_width = tmp_window_width;
        window_height = tmp_window_height;

        GenerateScreenTexture();
    }
}

void PostprocessShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess.frag")}
    };
    shader_id = LoadShaders(shader_path_map);

    Use();
    SetInt("scene_texture", 0);
    assert(shader_id, "Shader load failed!");
    
    Engine engine = Engine::GetEngine();
    window_width = engine.GetWindowWidth();
    window_height = engine.GetWindowHeight();
    
    glGenFramebuffers(1, &screen_quad_fbo);
    GenerateScreenTexture();
    
    // 屏幕mesh
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    
    // 初始化屏幕相关的 VAO
    glGenVertexArrays(1, &screen_quad_vao);
    glGenBuffers(1, &screen_quad_vbo);
    glBindVertexArray(screen_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void PostprocessShader::GenerateScreenTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, screen_quad_fbo);
    if(!screen_quad_texture)
    {
        glGenTextures(1, &screen_quad_texture);    
    }
    
    glBindTexture(GL_TEXTURE_2D, screen_quad_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, window_width, window_height,
    0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // depth buffer
    if(!scene_rbo)
    {
        // 注意这里不是glGenTextures，搞错了查了半天
        glGenRenderbuffers(1, &scene_rbo);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, scene_rbo);
    
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_quad_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, scene_rbo);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
	
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
