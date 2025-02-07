#include "WaterRenderSystem.hpp"

#include "Actor.hpp"
#include "RenderModule.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include "Component/Mesh/GerstnerWaveWater.h"
#include "Component/Mesh/Water.h"

namespace Kong
{
    class GerstnerWaveWater;
    class Water;
}

using namespace Kong;

void WaterRenderSystem::Init()
{
    auto window_size = KongWindow::GetWindowModule().windowSize;
    // water相关的buffer初始化
    glGenFramebuffers(1, &water_reflection_fbo);
    glGenFramebuffers(1, &water_refraction_fbo);

    GenerateWaterRenderTextures(window_size.x, window_size.y);
}

RenderResultInfo WaterRenderSystem::Draw(double delta, const RenderResultInfo& render_result_info,
    KongRenderModule* render_module)
{
    // 此时传入的render_result_info应该指向的是main framebuffer
    // 有水体，需要做一次从下往上的渲染获取反射的内容
    if (auto water_actor = m_waterActor.lock())
    {
        auto mainCamera = render_module->GetCamera();
        vec3 origin_cam_pos = mainCamera->GetPosition();
        // camera在水面之上
        float height_diff = origin_cam_pos.y - water_actor->location.y;
        if (height_diff > 0.0f)
        {
            // 复制普通场景渲染中postprocess的scene texture作为折射贴图使用
            ivec2 window_size = KongWindow::GetWindowModule().windowSize;
            glBindFramebuffer(GL_READ_FRAMEBUFFER, render_result_info.frameBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, water_refraction_fbo);
            glBlitFramebuffer(0, 0, window_size.x, window_size.y, 0, 0,
                window_size.x, window_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
            // 处理水面折射
            mainCamera->SetPosition(origin_cam_pos + vec3(0,-2*height_diff, 0));
            mainCamera->InvertPitch();
		
            render_module->matrix_ubo.Bind();
            render_module->matrix_ubo.UpdateData(mainCamera->GetViewMatrix(), "view");
            render_module->matrix_ubo.UpdateData(mainCamera->GetProjectionMatrix(), "projection");
            render_module->matrix_ubo.UpdateData(mainCamera->GetPosition(), "cam_pos");
            render_module->matrix_ubo.EndBind();
                        
            bool tmp_ssr = render_module->use_screen_space_reflection;
            render_module->use_screen_space_reflection = false;
            render_module->RenderSceneObject(water_reflection_fbo);
            render_module->use_screen_space_reflection = tmp_ssr;
					
            // 渲染水需要恢复相机位置
            mainCamera->SetPosition(origin_cam_pos);
            mainCamera->InvertPitch();
		
            render_module->matrix_ubo.Bind();
            render_module->matrix_ubo.UpdateData(mainCamera->GetViewMatrix(), "view");
            render_module->matrix_ubo.UpdateData(mainCamera->GetProjectionMatrix(), "projection");
            render_module->matrix_ubo.UpdateData(mainCamera->GetPosition(), "cam_pos");
            render_module->matrix_ubo.EndBind();
            
            DrawWater(delta, render_result_info, render_module);
        }
        else
        {
            // todo: 水下
        }
    }

    return render_result_info;
}

void WaterRenderSystem::DrawUI()
{
    KongRenderSystem::DrawUI();
}

void WaterRenderSystem::DrawWater(double delta, const RenderResultInfo& render_result_info,
    KongRenderModule* render_module)
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, render_result_info.frameBuffer);
    total_move += delta;
    if (m_waterActor.expired())
    {
        return;
    }

    std::shared_ptr<AActor> water_actor = m_waterActor.lock();
	
    // glEnable(GL_CLIP_DISTANCE0);
    // auto actors = CScene::GetActors();
    //for(auto actor : actors)
    auto water_comp = water_actor->GetComponent<Water>();
    if(water_comp)
    {
        auto mesh_shader = water_comp->shader_data;

        mesh_shader->Use();
        glBindTextureUnit(0, water_reflection_texture);
        glBindTextureUnit(1, water_refraction_texture);
		
        // 等于1代表渲染skybox，会需要用到环境贴图
        // mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
        mesh_shader->SetMat4("model", water_actor->GetModelMatrix());
        mesh_shader->SetFloat("move_factor",
            fmodf(total_move*move_speed, 1.0));
        water_comp->Draw(render_module->scene_render_info);
    }
    else
    {
        auto gerstner_water = water_actor->GetComponent<GerstnerWaveWater>();
        if(!gerstner_water)
        {
            return;
        }

        auto mesh_shader = gerstner_water->shader_data;

        mesh_shader->Use();
        glBindTextureUnit(0, water_reflection_texture);
        glBindTextureUnit(1, water_refraction_texture);

		
        // 等于1代表渲染skybox，会需要用到环境贴图
        // mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
        mesh_shader->SetMat4("model", water_actor->GetModelMatrix());
        mesh_shader->SetDouble("iTime", total_move);
        gerstner_water->Draw(render_module->scene_render_info);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WaterRenderSystem::GenerateWaterRenderTextures(int width, int height)
{
    // 水面反射相关的buffer
    glBindFramebuffer(GL_FRAMEBUFFER, water_reflection_fbo);
    TextureCreateInfo water_tex_create_info {
        GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT, width, height,
            GL_REPEAT, GL_REPEAT, GL_REPEAT, GL_LINEAR
        };
    TextureBuilder::CreateTexture(water_reflection_texture, water_tex_create_info);
	
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_reflection_texture, 0);
	
    if (!water_reflection_rbo)
    {
        glGenRenderbuffers(1, &water_reflection_rbo);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, water_reflection_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, water_reflection_rbo);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("water reflection framebuffer error\n");
    }

    // 水面折射相关的buffer
    glBindFramebuffer(GL_FRAMEBUFFER, water_refraction_fbo);

    TextureBuilder::CreateTexture(water_refraction_texture, water_tex_create_info);
	
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_refraction_texture, 0);
	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("water refraction framebuffer error\n");
    }
	
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
