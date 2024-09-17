#include "VolumetricCloud.h"

using namespace Kong;
CloudModel::CloudModel()
{
    perlin_worley_comp_shader = make_shared<Shader>();
    perlin_worley_comp_shader->Init({{gs,""}});
    
    worley_comp_shader = make_shared<Shader>();
    worley_comp_shader->Init({{gs,""}});

    cloud_compute_shader = make_shared<Shader>();
    cloud_compute_shader->Init({{gs,""}});

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
    glBindImageTexture(0, perlin_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    glGenTextures(1, &worley32_texture);
    glBindTexture(GL_TEXTURE_3D, worley32_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 32, 32, 32, 0, GL_RGBA, GL_FLOAT, NULL);

    glGenerateMipmap(GL_TEXTURE_3D);
    glBindImageTexture(0, worley32_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    
    glGenTextures(1, &weather_texutre);
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
