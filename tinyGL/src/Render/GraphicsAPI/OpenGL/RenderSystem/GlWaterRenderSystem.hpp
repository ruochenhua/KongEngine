#pragma once
#include <memory>

#include "OpenGLRenderSystem.hpp"

namespace Kong
{
    class KongTexture;
}

namespace Kong
{
    class AActor;

    class GlWaterRenderSystem : public OpenGLRenderSystem
    {
    public:
        void Init() override;
        RenderResultInfo Draw(double delta, const RenderResultInfo& render_result_info,
            KongRenderModule* render_module) override;
        
        void DrawUI() override;

        std::weak_ptr<AActor> m_waterActor;

    private:
        void DrawWater(double delta, const RenderResultInfo& render_result_info,
            KongRenderModule* render_module);
        
        // 反射部分，需要变换相机角度重新渲染
        GLuint water_reflection_fbo = GL_NONE;
        GLuint water_reflection_rbo = GL_NONE;
        // GLuint water_reflection_texture = GL_NONE;
        std::shared_ptr<KongTexture> water_reflection_texture;

        // 折射部分，先使用延迟渲染的结果复制过来
        GLuint water_refraction_fbo = GL_NONE;
        // GLuint water_refraction_texture = GL_NONE;
        std::shared_ptr<KongTexture> water_refraction_texture;
        
        void GenerateWaterRenderTextures(int width, int height);

        float total_move = 0.0f;
        float move_speed = 0.01f;
    };
}
