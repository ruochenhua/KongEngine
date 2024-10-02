#pragma once
#include "MeshComponent.h"

namespace Kong
{
    // 地形类
    class Terrain : public CMeshComponent
    {
    public:
        Terrain();
        Terrain(const string &file_name);
        void SimpleDraw() override;
        float height_scale_ = 64.0f;
        float height_shift_ = 16.0f;

        // perlin noise生成数据相关
        float amplitude = 12.f;
        int octaves = 8;
        float freq = 0.002f;
        float power = 2.0f;
        
        void Draw(const SSceneRenderInfo& scene_render_info) override;
        void InitRenderInfo() override;
        
    private:
        // 读取高度图
        int LoadHeightMap(const string &file_name);
        GLuint terrain_vao = GL_NONE;
        GLuint terrain_vbo = GL_NONE;
        GLuint terrain_ebo = GL_NONE;
        GLuint terrain_height_map = GL_NONE;
        
        std::vector<float> height_data;
        std::vector<unsigned int> height_indices;

        unsigned int num_strips = 0;
        unsigned int num_verts_per_strip = 0;
        int terrain_res = 100;
        bool render_wireframe = false;

        GLuint grass_albedo_texture = GL_NONE;
        GLuint grass_normal_texture = GL_NONE;

        GLuint sand_albedo_texture = GL_NONE;
        GLuint sand_normal_texture = GL_NONE;
        
        GLuint rock_albedo_texture = GL_NONE;
        GLuint rock_normal_texture = GL_NONE;
        
        int terrain_size = 10000;
    };
}
