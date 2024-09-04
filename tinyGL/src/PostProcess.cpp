#include "PostProcess.h"
using namespace Kong;

void PostProcess::Init()
{
    Engine engine = Engine::GetEngine();
    glm::ivec2 window_size = engine.GetWindowSize();
    window_width = window_size.x;
    window_height = window_size.y;
    
    final_postprocess = make_shared<FinalPostprocessShader>();
    final_postprocess->InitPostProcessShader(window_width, window_height);
    gaussian_blur = make_shared<GaussianBlurShader>();
    gaussian_blur->InitPostProcessShader(window_width, window_height);
    
    InitQuad();
    InitScreenTexture();
}

void PostProcess::OnWindowResize(unsigned width, unsigned height)
{
    window_width = width;
    window_height = height;

    // 删掉并释放掉原来的贴图资源
    glDeleteTextures(2, screen_quad_texture);
    screen_quad_texture[0] = screen_quad_texture[1] = GL_NONE;
    glDeleteRenderbuffers(1, &scene_rbo);
    scene_rbo = GL_NONE;
    // 重新创建
    InitScreenTexture();

    gaussian_blur->GenerateTexture(window_width, window_height);
}

void PostProcess::Draw()
{
    if(bloom)
    {
        gaussian_blur->SetBlurAmount(bloom_range);
        auto blur_textures = gaussian_blur->Draw({screen_quad_texture[1]},
            screen_quad_vao);
        final_postprocess->Draw({screen_quad_texture[0], blur_textures[0]}, screen_quad_vao);
    }
    else
    {
        final_postprocess->Draw({screen_quad_texture[0]}, screen_quad_vao);
    }
}

void PostProcess::InitQuad()
{
    glGenFramebuffers(1, &screen_quad_fbo);
    
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

void PostProcess::InitScreenTexture()
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // 渲染到两个颜色附件上
    GLuint color_attachment[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};  
    glDrawBuffers(2, color_attachment); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
