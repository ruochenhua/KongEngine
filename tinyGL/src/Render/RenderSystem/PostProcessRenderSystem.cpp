#include "PostProcessRenderSystem.hpp"
#ifndef RENDER_IN_VULKAN
#include <imgui.h>
#endif
#include "Render/RenderModule.hpp"
#include "Render/Resource/Texture.hpp"
#include "Window.hpp"
using namespace Kong;

PostProcessRenderSystem::PostProcessRenderSystem()
{
    m_Type = RenderSystemType::POST_PROCESS;
}

void PostProcessRenderSystem::Init()
{
    glm::ivec2 window_size = KongWindow::GetWindowModule().windowSize;
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
}

void PostProcessRenderSystem::OnWindowResize(unsigned width, unsigned height)
{
    window_width = width;
    window_height = height;

    combine_process->GenerateTexture(window_width, window_height);
    gaussian_blur->GenerateTexture(window_width, window_height);
    dilate_blur->GenerateTexture(window_width, window_height);
    dof_process->GenerateTexture(window_width, window_height);
    radical_blur->GenerateTexture(window_width, window_height);
}

RenderResultInfo PostProcessRenderSystem::Draw(double delta, const RenderResultInfo& render_result_info,
    KongRenderModule* render_module)
{
    // post process往屏幕上（frame buffer 0）渲染
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // 渲染到屏幕的texture
    // 0: 正常场景；1：bloom颜色；2：反射颜色
    auto& screen_quad_texture = render_module->m_renderToTextures;
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
        postprocess_rst = dof_process->Draw({postprocess_rst, dilate_textures, render_result_info.resultPosition}, screen_quad_vao);
    }


    final_postprocess->Draw({postprocess_rst}, screen_quad_vao);

    
    return render_result_info;
}

void PostProcessRenderSystem::DrawUI()
{
#ifndef RENDER_IN_VULKAN
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
#endif
    
}

void PostProcessRenderSystem::InitQuad()
{
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
