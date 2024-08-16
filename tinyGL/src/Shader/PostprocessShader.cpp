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
    bool horizontal = true, first_iteration = true;
    int amount = 10;
    glUseProgram(blur_shader_id);
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo[horizontal]);
        //SetInt("horizontal", horizontal);
        glUniform1i(glGetUniformLocation(blur_shader_id, "horizontal"), horizontal);
        glActiveTexture(GL_TEXTURE0);
        // bind texture of other framebuffer (or scene if first iteration)
        glBindTexture(GL_TEXTURE_2D, first_iteration ? screen_quad_texture[1] : blur_texture[!horizontal]);
        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(GL_NONE);
        
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glBlendFunc(GL_ONE, GL_ONE);
    Use();
    glBindVertexArray(screen_quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_quad_texture[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blur_texture[!horizontal]);
    
    auto main_cam = CRender::GetRender()->GetCamera();
    if(main_cam)
    {
        SetFloat("exposure", main_cam->exposure);    
    }
    SetBool("bloom", bloom);
    
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
    assert(shader_id, "Shader load failed!");
    
    Use();
    SetInt("scene_texture", 0);
    SetInt("bright_texture", 1);
    map<EShaderType, string> blur_shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/postprocess/gaussian_blur.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/postprocess/gaussian_blur.frag")}
    };
    blur_shader_id = LoadShaders(blur_shader_path_map);
    assert(blur_shader_id, "Shader load failed!");
    glUseProgram(blur_shader_id);
    glUniform1i(glGetUniformLocation(blur_shader_id, "bright_texture"), 0); 
    
    Engine engine = Engine::GetEngine();
    window_width = engine.GetWindowWidth();
    window_height = engine.GetWindowHeight();
    
    glGenFramebuffers(1, &screen_quad_fbo);
    glGenFramebuffers(2, blur_fbo);
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
    if(!screen_quad_texture[0])
    {
        glGenTextures(2, screen_quad_texture);    
    }

    for(unsigned i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, screen_quad_texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height,
        0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, screen_quad_texture[i], 0);
    }
    // depth buffer
    if(!scene_rbo)
    {
        // 注意这里不是glGenTextures，搞错了查了半天
        glGenRenderbuffers(1, &scene_rbo);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, scene_rbo);
    
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, scene_rbo);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    // 渲染到两个颜色附件上
    GLuint color_attachment[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};  
    glDrawBuffers(2, color_attachment); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(!blur_fbo[0])
    {
        glGenFramebuffers(2, blur_fbo);
    }
    if(!blur_texture[0])
    {
        glGenTextures(2, blur_texture);
    }
    
    for(unsigned i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo[i]);

        glBindTexture(GL_TEXTURE_2D, blur_texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height,
        0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_texture[i], 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
