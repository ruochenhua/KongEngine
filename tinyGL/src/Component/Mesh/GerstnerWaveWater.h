
#pragma once
#include "MeshComponent.h"

namespace Kong
{
    class GerstnerWaveWater : public CMeshComponent
    {
    public:
        GerstnerWaveWater();
        void Draw(void* commandBuffer = nullptr) override;
        void DrawShadowInfo(shared_ptr<Shader> simple_draw_shader) override;
        void InitRenderInfo() override;
        
        float height_scale_ = 1.0f;
        float height_shift_ = 0.0f;
        std::vector<float> height_data;
        std::vector<unsigned int> height_indices;
        
        int water_size = 100;
        int water_resolution = 10;

        // // gerstner wave相关参数
        // float amplitude = 5.f;
        // int octaves = 8;
        // float freq = 0.002f;
        // float power = 2.0f;

        bool render_wireframe = false;

        // todo: 也许应该继承于water类型
        void LoadDudvMapTexture(const string& texture_path);
        void LoadNormalTexture(const string& texture_path);
    private:
        // dudv贴图
        GLuint dudv_texture = 0;
        // normal map
        GLuint normal_texture = 0;
        
        GLuint gerstner_wave_vao = GL_NONE;
        GLuint gerstner_wave_vbo = GL_NONE;
    };
}
