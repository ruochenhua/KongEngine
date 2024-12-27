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

        void LoadDudvMapTexture(const string& texture_path);
    private:
		// dudv贴图
        GLuint dudv_texture = 0;
    };    
}

