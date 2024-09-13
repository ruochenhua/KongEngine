#include "Terrain.h"

#include "render.h"
#include "stb_image.h"

using namespace Kong;
#define USE_TCS 1

Terrain::Terrain()
{
#if USE_TCS
    map<EShaderType, string> shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.frag")},
        {tcs, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.tesc")},
        {tes, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.tese")}
    };
#else
    map<EShaderType, string> shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/terrain/terrain_cpu.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/terrain/terrain_cpu.frag")},
    };
#endif

    string grass_dir = "textures/terrain/grass/";
    string rock_snow_dir = "textures/terrain/rock_snow/";
    string sand_dir = "textures/terrain/sand/";
    
    string grass_albedo_path = grass_dir + "stylized-grass1_albedo.png";
    string grass_normal_path = grass_dir + "stylized-grass1_normal-ogl.png";

    grass_albedo_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(grass_albedo_path));
    grass_normal_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(grass_normal_path));
    
    string rock_albedo_path = rock_snow_dir + "rock-snow-ice1-2k_Base_Color.png";
    string rock_normal_path = rock_snow_dir + "rock-snow-ice1-2k_Normal-ogl.png";
    rock_albedo_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(rock_albedo_path));
    rock_normal_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(rock_normal_path));
    
    string sand_albedo_path = sand_dir + "wavy-sand_albedo.png";
    string sand_normal_path = sand_dir + "wavy-sand_normal-ogl.png";
    sand_albedo_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(sand_albedo_path));
    sand_normal_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(sand_normal_path));
    
    shader_data = make_shared<Shader>(shader_path_map);
    shader_data->Use();
    shader_data->SetInt("height_map", 0);
    shader_data->SetInt("grass_texture", 1);
    shader_data->SetInt("grass_normal_texture", 2);
    shader_data->SetInt("sand_texture", 3);
    shader_data->SetInt("sand_normal_texture", 4);
    shader_data->SetInt("rock_texture", 5);
    shader_data->SetInt("rock_normal_texture", 6);
}

Terrain::Terrain(const string& file_name)
    :Terrain()
{
    LoadHeightMap(file_name);
}

void Terrain::SimpleDraw()
{
#if USE_TCS
    GLuint height_map_id = terrain_height_map > 0 ? terrain_height_map : CRender::GetNullTexId();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, height_map_id);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, grass_albedo_texture);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, grass_normal_texture);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sand_albedo_texture);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, sand_normal_texture);
    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, rock_albedo_texture);
    
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, rock_normal_texture);
    
    if(render_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0);
    }
    
    glDrawArrays(GL_PATCHES, 0, 4*rez*rez);
    if(render_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    
    }

#else
    for(unsigned int strip = 0; strip < num_strips; ++strip)
    {
        glDrawElements(GL_LINE_STRIP,
            num_verts_per_strip+2, GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * strip * (num_verts_per_strip+2)));
    }
#endif
}

void Terrain::Draw(const SSceneRenderInfo& scene_render_info)
{
    glDisable(GL_CULL_FACE);
    shader_data->Use();
    glBindVertexArray(terrain_vao);
    shader_data->SetInt("octaves", octaves);
    shader_data->SetFloat("amplitude", amplitude);
    shader_data->SetFloat("freq", freq);
    shader_data->SetFloat("power", power);
    shader_data->SetFloat("height_scale", height_scale_);
    shader_data->SetFloat("height_shift", height_shift_);
    SimpleDraw();
    
    glEnable(GL_CULL_FACE);
}

void Terrain::InitRenderInfo()
{
    // 依次读入方形四角的四个点，每四个点（方形）作为一个patch
    for(unsigned i = 0; i < rez; i++)
    {
        for(unsigned j = 0; j < rez; j++)
        {
            height_data.push_back(-terrain_width/2.f + terrain_width*i/(float)rez);     // x
            height_data.push_back(0.0);                                 // y
            height_data.push_back(-terrain_height/2.f + terrain_height*j/(float)rez);   // z
            height_data.push_back(i/(float)rez);                        // u
            height_data.push_back(j/(float)rez);                        // v

            height_data.push_back(-terrain_width/2.f + terrain_width*(i+1)/(float)rez); // x
            height_data.push_back(0.0);                                 // y
            height_data.push_back(-terrain_height/2.f + terrain_height*j/(float)rez);   // z
            height_data.push_back((i+1)/(float)rez);                    // u
            height_data.push_back(j/(float)rez);                        // v

            height_data.push_back(-terrain_width/2.f + terrain_width*i/(float)rez);         // x
            height_data.push_back(0.0);                                     // y
            height_data.push_back(-terrain_height/2.f + terrain_height*(j+1)/(float)rez);   // z
            height_data.push_back(i/(float)rez);                            // u
            height_data.push_back((j+1)/(float)rez);                        // v

            height_data.push_back(-terrain_width/2.f + terrain_width*(i+1)/(float)rez);     // x
            height_data.push_back(0.0);                                     // y
            height_data.push_back(-terrain_height/2.f + terrain_height*(j+1)/(float)rez);   // z
            height_data.push_back((i+1)/(float)rez);                        // u
            height_data.push_back((j+1)/(float)rez);                        // v
        }
    }

    if(terrain_vao) glDeleteBuffers(1, &terrain_vao);
    if(terrain_vbo) glDeleteBuffers(1, &terrain_vbo);
    if(terrain_ebo) glDeleteBuffers(1, &terrain_ebo);

    glGenVertexArrays(1, &terrain_vao);
    glBindVertexArray(terrain_vao);

    glGenBuffers(1, &terrain_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, terrain_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        height_data.size() * sizeof(float),
        &height_data[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
#if USE_TCS
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)) );
    glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
#else
    glGenBuffers(1, &terrain_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        height_indices.size() * sizeof(unsigned int),
        &height_indices[0], GL_STATIC_DRAW);
#endif
}

int Terrain::LoadHeightMap(const string& file_name)
{
    int nrChannels;
    unsigned char *data = stbi_load(file_name.c_str(), &terrain_width, &terrain_height, &nrChannels, 0);
#if USE_TCS
    if(terrain_height_map) glDeleteTextures(1, &terrain_height_map);
    glGenTextures(1, &terrain_height_map);
    glBindTexture(GL_TEXTURE_2D, terrain_height_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, terrain_width, terrain_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
#else
    
    rez = 1;
    for(unsigned int i = 0; i < height; i++)
    {
        for(unsigned int j = 0; j < width; j++)
        {
            unsigned char* texel = data + (j + width * i) * nrChannels;
            unsigned char y = texel[0];

            height_data.push_back(-height/2.0f + i);
            height_data.push_back((int)y*y_scale - y_shift);
            height_data.push_back(-width/2.0f + j);
        }
    }
    
    for(unsigned int i = 0; i < terrain_height-1; i+=rez)
    {
        for(unsigned int j = 0; j < terrain_width; j+=rez)
        {
            for(unsigned int k = 0; k < 2; k++)
            {
                height_indices.push_back(j + terrain_width * (i+k));
            }
        }
    }

    num_strips = (terrain_height - 1)/rez;
    num_verts_per_strip = (terrain_width/rez) * 2 - 2;
#endif
    
    stbi_image_free(data);
    return 1;
}
