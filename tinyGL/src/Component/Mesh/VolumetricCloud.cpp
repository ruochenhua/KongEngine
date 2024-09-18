#include "VolumetricCloud.h"

using namespace Kong;
CloudModel::CloudModel()
{
    perlin_worley_comp_shader = make_shared<Shader>();
    perlin_worley_comp_shader->Init(
        {{gs,CSceneLoader::ToResourcePath("shader/volumetric_cloud/perlin_worley.comp")}});

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

    
    worley_comp_shader = make_shared<Shader>();
    worley_comp_shader->Init(
        {{gs,CSceneLoader::ToResourcePath("shader/volumetric_cloud/worley.comp")}});

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

    
    weather_compute_shader = make_shared<Shader>();
    weather_compute_shader->Init(
        {{gs,CSceneLoader::ToResourcePath("shader/volumetric_cloud/weather.comp")}});
    // weather 材质
    glGenTextures(1, &weather_texutre);
    glBindTexture(GL_TEXTURE_2D, weather_texutre);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, weather_texutre, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    weather_compute_shader->Use();
    // weather_compute_shader->SetVec3("seed", seed);
    weather_compute_shader->SetFloat("perlin_frequency", perlin_frequency);
    glDispatchCompute(128, 128, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

VolumetricCloud::VolumetricCloud()
{
    map<EShaderType, string> shader_map = {
        {vs, ""},
        {fs, ""}
    };

    
}

void VolumetricCloud::SimpleDraw()
{

}
