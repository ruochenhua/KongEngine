#pragma once
#include "QuadShape.h"
#include "Render/Resource/Texture.hpp"

namespace Kong
{
    // 水，形状上是一个quad mesh，所以先继承于QuadShape
    class Water : public CQuadShape
    {
    public:
        Water();

        void DrawShadowInfo(shared_ptr<OpenGLShader> simple_draw_shader) override;
        
        void Draw(void* commandBuffer = nullptr) override;

        void LoadDudvMapTexture(const string& texture_path);
        void LoadNormalTexture(const string& texture_path);
    private:
		// dudv贴图
        std::weak_ptr<KongTexture> dudv_texture;
        // normal map
        std::weak_ptr<KongTexture> normal_texture;
    };    
}

