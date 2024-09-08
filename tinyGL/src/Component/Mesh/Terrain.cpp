#include "Terrain.h"

#include "stb_image.h"

using namespace Kong;

Terrain::Terrain(const string& file_name)
{
    ImportTerrain(file_name);
    map<EShaderType, string> shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/terrain.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/terrain.frag")},
    };

    terrain_shader = make_shared<Shader>(shader_path_map);
}

void Terrain::Draw()
{
    glDisable(GL_CULL_FACE);
    terrain_shader->Use();
    glBindVertexArray(terrain_vao);
    for(unsigned int strip = 0; strip < num_strips; ++strip)
    {
        glDrawElements(GL_TRIANGLE_STRIP,
            num_verts_per_strip+2, GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * strip * (num_verts_per_strip+2)));
    }
}

int Terrain::ImportTerrain(const string& file_name)
{
    int width, height, nrChannels;
    unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
    int rez = 1;
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
    stbi_image_free(data);
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrain_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        height_indices.size() * sizeof(unsigned int),
        &height_indices[0], GL_STATIC_DRAW);
    return 1;
}
