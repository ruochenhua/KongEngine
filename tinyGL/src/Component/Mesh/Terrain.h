#pragma once
#include "MeshComponent.h"

#define USE_TCS 1
namespace Kong
{
    // 地形类
    class Terrain : public CMeshComponent
    {
    public:
        Terrain();
        Terrain(const string &file_name);
        void DrawShadowInfo(shared_ptr<Shader> simple_draw_shader) override;
        float height_scale_ = 64.0f;
        float height_shift_ = 16.0f;

        // perlin noise生成数据相关
        float amplitude = 12.f;
        int octaves = 8;
        float freq = 0.002f;
        float power = 2.0f;
        
        void Draw(const SSceneLightInfo& scene_render_info) override;
        void InitRenderInfo() override;
        
        int terrain_size = 10000;
        int terrain_res = 100;
        
    private:
        // 读取高度图
        int LoadHeightMap(const string &file_name);
        GLuint terrain_vao = GL_NONE;
        GLuint terrain_vbo = GL_NONE;
        
        GLuint terrain_height_map = GL_NONE;
        
        std::vector<float> height_data;
        std::vector<unsigned int> height_indices;
        
#if !USE_TCS
        unsigned int num_strips = 0;
        unsigned int num_verts_per_strip = 0;
#endif
        
        bool render_wireframe = false;

        GLuint grass_albedo_texture = GL_NONE;
        GLuint grass_normal_texture = GL_NONE;

        GLuint sand_albedo_texture = GL_NONE;
        GLuint sand_normal_texture = GL_NONE;
        
        GLuint rock_albedo_texture = GL_NONE;
        GLuint rock_normal_texture = GL_NONE;
    };
}
