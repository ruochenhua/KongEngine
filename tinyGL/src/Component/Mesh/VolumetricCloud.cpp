#include "VolumetricCloud.h"
#ifndef RENDER_IN_VULKAN
#include <imgui.h>
#endif
#include "Utils.hpp"
#include "Render/RenderModule.hpp"
#include "Scene.hpp"
#include "Window.hpp"
#include "Component/LightComponent.h"
const int CLOUD_REZ = 16;
using namespace Kong;
CloudModel::CloudModel()
{
    perlin_worley_comp_shader = make_shared<OpenGLShader>();
    perlin_worley_comp_shader->Init(
        {{cs,CSceneLoader::ToResourcePath("shader/volumetric_cloud/perlin_worley.comp")}});

    // perlin noise 3d材质
    glGenTextures(1, &perlin_texture);
    glBindTexture(GL_TEXTURE_3D, perlin_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 128, 128, 128, 0, GL_RGBA, GL_FLOAT, NULL);

    glGenerateMipmap(GL_TEXTURE_3D);
    // glBindImageTexture(0, perlin_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    // 用compute shader生成3d perlin noise材质
    perlin_worley_comp_shader->Use();
    // perlin_worley_comp_shader->SetVec3("u_resolution", glm::vec3(128));
    perlin_worley_comp_shader->SetInt("out_vol_tex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, perlin_texture);
    glBindImageTexture(0, perlin_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glDispatchCompute(32, 32, 32);
    glGenerateMipmap(GL_TEXTURE_3D);

    
    worley_comp_shader = make_shared<OpenGLShader>();
    worley_comp_shader->Init(
        {{cs,CSceneLoader::ToResourcePath("shader/volumetric_cloud/worley.comp")}});

    // worley 3d材质
    glGenTextures(1, &worley32_texture);
    glBindTexture(GL_TEXTURE_3D, worley32_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 32, 32, 32, 0, GL_RGBA, GL_FLOAT, NULL);

    glGenerateMipmap(GL_TEXTURE_3D);
//    glBindImageTexture(0, worley32_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    worley_comp_shader->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, worley32_texture);
    glBindImageTexture(0, worley32_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glDispatchCompute(8, 8, 8);
    glGenerateMipmap(GL_TEXTURE_3D);

    
    weather_compute_shader = make_shared<OpenGLShader>();
    weather_compute_shader->Init(
        {{cs,CSceneLoader::ToResourcePath("shader/volumetric_cloud/weather.comp")}});
    // weather 材质
    glGenTextures(1, &weather_texutre);
    glBindTexture(GL_TEXTURE_2D, weather_texutre);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 2048, 2048, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, weather_texutre, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    GenerateWeatherMap();
}

void CloudModel::GenerateWeatherMap()
{
    weather_compute_shader->Use();
    // weather_compute_shader->SetVec3("seed", seed);
    weather_compute_shader->SetFloat("perlin_frequency", perlin_frequency);
    glDispatchCompute(128, 128, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

VolumetricCloud::VolumetricCloud()
    :cloud_tex(0), bloom_tex(0), alphaness_tex(0), depth_tex(0)
{
    cloud_model_ = make_shared<CloudModel>();
    cloud_compute_shader_ = make_shared<OpenGLShader>();
    cloud_compute_shader_->Init(
      {{cs,CSceneLoader::ToResourcePath("shader/volumetric_cloud/volumetric_cloud.comp")}});

    // 创建cloud相关texture并绑定
    glm::ivec2 window_size = KongWindow::GetWindowModule().windowSize;
    unsigned width = window_size.x;
    unsigned height = window_size.y;

    // todo: 窗口重设大小时需要重新设置材质的大小
    cloud_tex = GenerateTexture2D(width, height);
    bloom_tex = GenerateTexture2D(width, height);
    alphaness_tex = GenerateTexture2D(width, height);
    depth_tex = GenerateTexture2D(width, height);
}

void VolumetricCloud::PreRenderUpdate() const
{
#ifndef RENDER_IN_VULKAN
	ImGui::Begin("Volumetric Cloud");
	ImGui::PushItemWidth(80);
	ImGui::DragFloat("Coverage", &cloud_model_->coverage, 0.02f, 0.0, 3.0);
	ImGui::DragFloat("Cloud Speed", &cloud_model_->cloud_speed, 1.0f, 0.0f, 3000.0f);
	ImGui::DragFloat("Crispiness", &cloud_model_->crispiness, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("curliness", &cloud_model_->curliness, 0.02f, 0.0f, 30.0f);
	ImGui::DragFloat("Density", &cloud_model_->density, 0.001f, 0.0f, 1.f);
	ImGui::DragFloat("absorption", &cloud_model_->absorption, 0.02f, 0.0f, 5.0f);
	ImGui::DragFloat("perlin frequency", &cloud_model_->perlin_frequency, 0.04f, 0.0f, 10.0f);
	
	ImGui::End();
#endif
}

void VolumetricCloud::DrawShadowInfo(shared_ptr<OpenGLShader> simple_shader)
{
    // 计算cloud texture
	// 有太阳光才计算这个，没有就跳过
    auto dir_light = KongRenderModule::GetRenderModule().scene_render_info.scene_dirlight;
	if(dir_light.expired())
	{
		return;
	}

	auto dir_light_ptr = dir_light.lock();
	
	glm::ivec2 window_size = KongWindow::GetWindowModule().windowSize;
    glBindImageTexture(0, cloud_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, bloom_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, alphaness_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(3, depth_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    cloud_compute_shader_->Use();
	cloud_compute_shader_->SetVec2("iResolution", glm::vec2(window_size.x, window_size.y));
	cloud_compute_shader_->SetFloat("iTime", glfwGetTime());
	
	cloud_compute_shader_->SetVec3("lightDirection", dir_light_ptr->GetLightDir());
	cloud_compute_shader_->SetVec3("lightColor", dir_light_ptr->light_color);
	
	cloud_compute_shader_->SetFloat("coverage_multiplier", cloud_model_->coverage);
	cloud_compute_shader_->SetFloat("cloudSpeed", cloud_model_->cloud_speed);
	cloud_compute_shader_->SetFloat("crispiness", cloud_model_->crispiness);
	cloud_compute_shader_->SetFloat("curliness", cloud_model_->curliness);
	cloud_compute_shader_->SetFloat("absorption", cloud_model_->absorption*0.01);
	cloud_compute_shader_->SetFloat("densityFactor", cloud_model_->density);

	//cloud_compute_shader_.setBool("enablePowder", enablePowder);
	
	cloud_compute_shader_->SetFloat("earthRadius", cloud_model_->earth_radius);
	cloud_compute_shader_->SetFloat("sphereInnerRadius", cloud_model_->sphere_inner_radius);
	cloud_compute_shader_->SetFloat("sphereOuterRadius", cloud_model_->sphere_outer_radius);

	cloud_compute_shader_->SetVec3("cloudColorTop", cloud_model_->cloud_color_top);
	cloud_compute_shader_->SetVec3("cloudColorBottom", cloud_model_->cloud_color_bottom);
	
	cloud_compute_shader_->SetVec3("skyColorTop", cloud_model_->sky_color_top);
	cloud_compute_shader_->SetVec3("skyColorBottom", cloud_model_->sky_color_bottom);

	cloud_compute_shader_->SetInt("cloud", 0);
	cloud_compute_shader_->SetInt("worley32", 1);
	cloud_compute_shader_->SetInt("weatherTex", 2);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, cloud_model_->perlin_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, cloud_model_->worley32_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, cloud_model_->weather_texutre);
	
	//actual draw
	glDispatchCompute(window_size.x/CLOUD_REZ, window_size.y/CLOUD_REZ, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

GLuint VolumetricCloud::GenerateTexture2D(unsigned w, unsigned h)
{
    unsigned int tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    return tex_output;
}
