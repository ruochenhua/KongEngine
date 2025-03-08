
#include "GerstnerWaveWater.h"

#include "Render/RenderModule.hpp"
#include "Scene.hpp"
#include "Component/LightComponent.h"

using namespace Kong;

GerstnerWaveWater::GerstnerWaveWater()
{
    map<EShaderType, string> shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/water/gerstner_wave.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/water/gerstner_wave.frag")},
        {tcs, CSceneLoader::ToResourcePath("shader/water/gerstner_wave.tesc")},
        {tes, CSceneLoader::ToResourcePath("shader/water/gerstner_wave.tese")}
    };

    shader_data = make_shared<OpenGLShader>(shader_path_map);

    shader_data->Use();
    shader_data->SetInt("reflection_texture", 0);
    shader_data->SetInt("refraction_texture", 1);
    shader_data->SetInt("dudv_map", 2);
    shader_data->SetInt("normal_map", 3);
}

void GerstnerWaveWater::Draw(void* commandBuffer)
{
    // glDisable(GL_CULL_FACE);
    shader_data->Use();    
    glBindVertexArray(gerstner_wave_vao);
    shader_data->SetFloat("height_scale", height_scale_);
    shader_data->SetFloat("height_shift", height_shift_);
    shader_data->SetInt("terrain_size", water_size);
    shader_data->SetInt("terrain_res", water_resolution);

    if (!dudv_texture.expired()) dudv_texture.lock()->Bind(2);
    if (!normal_texture.expired()) normal_texture.lock()->Bind(3);
    
    DrawShadowInfo(nullptr);
}

void GerstnerWaveWater::DrawShadowInfo(shared_ptr<OpenGLShader> simple_draw_shader)
{
    shader_data->SetVec3("cam_pos", KongRenderModule::GetRenderModule().GetCamera()->GetPosition());
    // render_wireframe = true;
    
    if(render_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0);
    }
    
    glDrawArrays(GL_PATCHES, 0, 4*water_resolution*water_resolution);
    
    if(render_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    
    }
}

void GerstnerWaveWater::InitRenderInfo()
{
    // 依次读入方形四角的四个点，每四个点（方形）作为一个patch
    for(unsigned i = 0; i < water_resolution; i++)
    {
        for(unsigned j = 0; j < water_resolution; j++)
        {
            height_data.push_back(-water_size/2.f + water_size*i/(float)water_resolution);     // x
            height_data.push_back(0.0);                                 // y
            height_data.push_back(-water_size/2.f + water_size*j/(float)water_resolution);   // z
            height_data.push_back(i/(float)water_resolution);                        // u
            height_data.push_back(j/(float)water_resolution);                        // v

            height_data.push_back(-water_size/2.f + water_size*(i+1)/(float)water_resolution); // x
            height_data.push_back(0.0);                                 // y
            height_data.push_back(-water_size/2.f + water_size*j/(float)water_resolution);   // z
            height_data.push_back((i+1)/(float)water_resolution);                    // u
            height_data.push_back(j/(float)water_resolution);                        // v

            height_data.push_back(-water_size/2.f + water_size*i/(float)water_resolution);         // x
            height_data.push_back(0.0);                                     // y
            height_data.push_back(-water_size/2.f + water_size*(j+1)/(float)water_resolution);   // z
            height_data.push_back(i/(float)water_resolution);                            // u
            height_data.push_back((j+1)/(float)water_resolution);                        // v

            height_data.push_back(-water_size/2.f + water_size*(i+1)/(float)water_resolution);     // x
            height_data.push_back(0.0);                                     // y
            height_data.push_back(-water_size/2.f + water_size*(j+1)/(float)water_resolution);   // z
            height_data.push_back((i+1)/(float)water_resolution);                        // u
            height_data.push_back((j+1)/(float)water_resolution);                        // v
        }
    }

    if(gerstner_wave_vao) glDeleteBuffers(1, &gerstner_wave_vao);
    if(gerstner_wave_vbo) glDeleteBuffers(1, &gerstner_wave_vbo);

    glGenVertexArrays(1, &gerstner_wave_vao);
    glBindVertexArray(gerstner_wave_vao);

    glGenBuffers(1, &gerstner_wave_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gerstner_wave_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        height_data.size() * sizeof(float),
        &height_data[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)) );
    glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

}

void GerstnerWaveWater::LoadDudvMapTexture(const string& texture_path)
{
    // 类型对opengl没区别
    dudv_texture = ResourceManager::GetOrLoadTexture_new(diffuse, CSceneLoader::ToResourcePath(texture_path));
}

void GerstnerWaveWater::LoadNormalTexture(const string& texture_path)
{
    // 类型对opengl没区别
    normal_texture = ResourceManager::GetOrLoadTexture_new(normal, CSceneLoader::ToResourcePath(texture_path));
}

