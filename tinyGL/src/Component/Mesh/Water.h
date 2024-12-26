#pragma once
#include "QuadShape.h"

namespace Kong
{
    // 水，形状上是一个quad mesh，所以先继承于QuadShape
    class Water : public CQuadShape
    {
    public:
        Water();

        void SimpleDraw(shared_ptr<Shader> simple_draw_shader) override;
        
        void Draw(const SSceneLightInfo& scene_render_info) override;

    private:
        // 水相关的渲染数据
        GLuint render_scene_texture = 0;
        
        GLuint refraction_texture = 0;
        GLuint reflection_texture = 0;
        GLuint dudv_texture = 0;
    };    
}

