#include "PostProcess.h"

#include <imgui.h>

#include "Engine.h"
using namespace Kong;

void PostProcess::Init()
{
    Engine engine = Engine::GetEngine();
    glm::ivec2 window_size = engine.GetWindowSize();
    window_width = window_size.x;
    window_height = window_size.y;

    combine_process = make_shared<CombineProcessShader>();
    combine_process->InitPostProcessShader(window_width, window_height);
    final_postprocess = make_shared<FinalPostprocessShader>();
    final_postprocess->InitPostProcessShader(window_width, window_height);
    gaussian_blur = make_shared<GaussianBlurShader>();
    gaussian_blur->InitPostProcessShader(window_width, window_height);
    dilate_blur = make_shared<DilatePostprocessShader>();
    dilate_blur->InitPostProcessShader(window_width, window_height);
    dof_process = make_shared<DOFPostprocessShader>();
    dof_process->InitPostProcessShader(window_width, window_height);
    radical_blur = make_shared<RadicalBlurShader>();
    radical_blur->InitPostProcessShader(window_width, window_height);
    
    InitQuad();
    InitScreenTexture();
}

void PostProcess::OnWindowResize(unsigned width, unsigned height)
{
    window_width = width;
    window_height = height;

    // 删掉并释放掉原来的贴图资源
    glDeleteTextures(PP_TEXTURE_COUNT, screen_quad_texture);
    for(int i = 0; i < PP_TEXTURE_COUNT; ++i)
    {
        screen_quad_texture[i] = GL_NONE;    
    }
    
    glDeleteRenderbuffers(1, &scene_rbo);
    scene_rbo = GL_NONE;
    // 重新创建
    InitScreenTexture();

    combine_process->GenerateTexture(window_width, window_height);
    gaussian_blur->GenerateTexture(window_width, window_height);
    dilate_blur->GenerateTexture(window_width, window_height);
    dof_process->GenerateTexture(window_width, window_height);
    radical_blur->GenerateTexture(window_width, window_height);
}

void PostProcess::Draw()
{
    // 先将屏幕空间反射和主场景渲染的内容整合
    combine_process->SetCombineMode(CombineProcessShader::Alpha);
    auto postprocess_rst = combine_process->Draw({screen_quad_texture[0], screen_quad_texture[2]}, screen_quad_vao);

    if(enable_bloom)
    {
        gaussian_blur->SetBlurAmount(bloom_range);
        auto blur_texture = gaussian_blur->Draw({screen_quad_texture[1]},
            screen_quad_vao);
        combine_process->SetCombineMode(CombineProcessShader::Add);
        postprocess_rst = combine_process->Draw({postprocess_rst, blur_texture}, screen_quad_vao);
    }

    if (enable_god_ray)
    {
        auto radical_blur_texture = radical_blur->Draw({screen_quad_texture[1]}, screen_quad_vao);
        postprocess_rst = radical_blur_texture;
        // combine_process->SetCombineMode(CombineProcessShader::Add);
        // postprocess_rst = combine_process->Draw({postprocess_rst, radical_blur_texture}, screen_quad_vao);
    }
    
    if(enable_DOF)
    {
        dilate_blur->SetParam(dilate_size, dilate_separation);
        
        auto dilate_textures = dilate_blur->Draw({postprocess_rst}, screen_quad_vao);
        dof_process->SetFocusDistance(focus_distance, focus_threshold);
        postprocess_rst = dof_process->Draw({postprocess_rst, dilate_textures, position_texture}, screen_quad_vao);
    }


    final_postprocess->Draw({postprocess_rst}, screen_quad_vao);
}

void PostProcess::RenderUI()
{
    ImGui::Begin("Post Process");
    ImGui::PushItemWidth(100);
    // 两个只开一个
    // dilate data
    ImGui::Checkbox("Depth of Field", &enable_DOF);
    ImGui::DragFloat("focus distance", &focus_distance, 0.1f, 0.1f, 100.0);
    ImGui::DragFloat("focus threshold min", &focus_threshold.x, 0.1f, 0.1f, focus_threshold.y);
    ImGui::DragFloat("focus threshold max", &focus_threshold.y, 0.1f, focus_threshold.x, 100.0);
    
    ImGui::DragInt("dilate size", &dilate_size, 0.1f, 0, 10);
    ImGui::DragFloat("dilate separation", &dilate_separation, 0.03f, 0.0, 5.0);
    
    // bloom data
    if(!enable_DOF)
    {
        ImGui::Checkbox("bloom", &enable_bloom);
        ImGui::DragInt("bloom range", &bloom_range, 1, 1, 500);
    }
    else
    {
        enable_bloom = false;
    }

    ImGui::Checkbox("god ray", &enable_god_ray);
    // todo god ray相关的参数
    ImGui::End();
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
        glGenTextures(PP_TEXTURE_COUNT, screen_quad_texture);    
    }

    for(unsigned i = 0; i < PP_TEXTURE_COUNT; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, screen_quad_texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height,
        0, GL_RGBA, GL_FLOAT, NULL);

        // 为啥一定要执行前两句？
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
    // 渲染到多个颜色附件上
    GLuint color_attachment[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};  
    glDrawBuffers(3, color_attachment); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
