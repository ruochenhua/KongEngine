#include "Terrain.h"

#include "stb_image.h"

using namespace Kong;
#define USE_TCS 1
Terrain::Terrain(const string& file_name)
{
    ImportTerrain(file_name);
    
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
    
    terrain_shader = make_shared<Shader>(shader_path_map);
    terrain_shader->SetInt("height_map", 0);
}

void Terrain::Draw()
{
    glDisable(GL_CULL_FACE);
    terrain_shader->Use();
    glBindVertexArray(terrain_vao);
#if USE_TCS
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrain_height_map);

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

int Terrain::ImportTerrain(const string& file_name)
{
    int width, height, nrChannels;
    unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#if USE_TCS
    if(terrain_height_map) glDeleteTextures(1, &terrain_height_map);
    glGenTextures(1, &terrain_height_map);
    glBindTexture(GL_TEXTURE_2D, terrain_height_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    // 依次读入方形四角的四个点，每四个点（方形）作为一个patch
    for(unsigned i = 0; i < rez; i++)
    {
        for(unsigned j = 0; j < rez; j++)
        {
            height_data.push_back(-width/2.f + width*i/(float)rez);     // x
            height_data.push_back(0.0);                                 // y
            height_data.push_back(-height/2.f + height*j/(float)rez);   // z
            height_data.push_back(i/(float)rez);                        // u
            height_data.push_back(j/(float)rez);                        // v

            height_data.push_back(-width/2.f + width*(i+1)/(float)rez); // x
            height_data.push_back(0.0);                                 // y
            height_data.push_back(-height/2.f + height*j/(float)rez);   // z
            height_data.push_back((i+1)/(float)rez);                    // u
            height_data.push_back(j/(float)rez);                        // v

            height_data.push_back(-width/2.f + width*i/(float)rez);         // x
            height_data.push_back(0.0);                                     // y
            height_data.push_back(-height/2.f + height*(j+1)/(float)rez);   // z
            height_data.push_back(i/(float)rez);                            // u
            height_data.push_back((j+1)/(float)rez);                        // v

            height_data.push_back(-width/2.f + width*(i+1)/(float)rez);     // x
            height_data.push_back(0.0);                                     // y
            height_data.push_back(-height/2.f + height*(j+1)/(float)rez);   // z
            height_data.push_back((i+1)/(float)rez);                        // u
            height_data.push_back((j+1)/(float)rez);                        // v
        }
    }
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
#endif
    
    for(unsigned int i = 0; i < height-1; i+=rez)
    {
        for(unsigned int j = 0; j < width; j+=rez)
        {
            for(unsigned int k = 0; k < 2; k++)
            {
                height_indices.push_back(j + width * (i+k));
            }
        }
    }

    num_strips = (height - 1)/rez;
    num_verts_per_strip = (width/rez) * 2 - 2;

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
#endif
    
    
    glGenBuffers(1, &terrain_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        height_indices.size() * sizeof(unsigned int),
        &height_indices[0], GL_STATIC_DRAW);

    
    stbi_image_free(data);
    return 1;
}
