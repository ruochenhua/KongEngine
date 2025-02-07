#pragma once
#include <memory>

#include "RenderSystem.hpp"

namespace Kong
{
    class AActor;

    class WaterRenderSystem : public KongRenderSystem
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
        GLuint water_reflection_texture = GL_NONE;

        // 折射部分，先使用延迟渲染的结果复制过来
        GLuint water_refraction_fbo = GL_NONE;
        GLuint water_refraction_texture = GL_NONE;
        
        void GenerateWaterRenderTextures(int width, int height);

        float total_move = 0.0f;
        float move_speed = 0.01f;
    };
}
