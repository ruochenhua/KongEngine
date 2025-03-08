#pragma once
#include <unordered_map>

#include "GlPostProcessRenderSystem.hpp"
#include "OpenGLRenderSystem.hpp"
#include "Component/Mesh/QuadShape.h"
#include "Shader/OpenGL/DeferInfoShader.h"

namespace Kong
{
    // ssao渲染相关
    struct SSAOHelper
    {
        unsigned ssao_kernel_count = 64;
        unsigned ssao_noise_size = 4;
        vector<glm::vec3> ssao_kernal_samples;
        vector<glm::vec3> ssao_kernal_noises;
		
        GLuint ssao_fbo = GL_NONE;
        GLuint SSAO_BlurFBO = GL_NONE;
        GLuint ssao_noise_texture = GL_NONE;
        GLuint ssao_result_texture = GL_NONE;
        GLuint ssao_blur_texture = GL_NONE;
        shared_ptr<SSAOShader> ssao_shader_;
        shared_ptr<OpenGLShader> ssao_blur_shader_;

        void Init(int width, int height);
        void GenerateSSAOTextures(int width, int height);
    };
    
    class GlDeferRenderSystem : public OpenGLRenderSystem
    {
    public:
        enum class DeferResType : uint8_t
        {
            Position    = 0,
            Normal,
            Albedo,
            Orm
        };

        GlDeferRenderSystem();
        
        void Init() override;

        RenderResultInfo Draw(double delta,
            const RenderResultInfo& render_result_info,
            KongRenderModule* render_module) override;

        void DrawUI() override;
        
        GLuint GetPositionTexture();
        GLuint GetNormalTexture();
        GLuint GetAlbedoTexture();
        GLuint GetOrmTexture();

        GLuint GetFrameBuffer() const;

        shared_ptr<DeferredBRDFShader> GetDeferredBRDFShader() const {return m_deferredBRDFShader;}
    protected:
        friend class KongRenderModule;
        
        shared_ptr<DeferredBRDFShader> m_deferredBRDFShader;
        std::unordered_map<DeferResType, GLuint> m_deferredResourceMap
        {
            {DeferResType::Position, GL_NONE},
            {DeferResType::Normal, GL_NONE},
            {DeferResType::Albedo, GL_NONE},
            {DeferResType::Orm, GL_NONE},
        };

        GLuint m_infoBuffer {0};        // 场景信息
        GLuint m_infoRbo {0};
                
        void GenerateDeferInfoTextures(int width, int height);

        void RenderToBuffer(KongRenderModule* render_module);
        void RenderToTexture(GLuint render_to_buffer, KongRenderModule* render_module);

        //ssao相关
        SSAOHelper m_ssaoHelper;
        void SSAORender();
        // 启用屏幕空间环境光遮蔽
        bool use_ssao = false;

        // 启用反射阴影贴图
        bool use_rsm = false;
        float rsm_intensity = 0.04f;
        int rsm_sample_count = 32;
		
        vector<glm::vec4> rsm_samples_and_weights;
        // 启用PCSS
        bool use_pcss = false;
        float pcss_radius = 1.0f;
        float pcss_light_scale = 0.1f;
        int pcss_sample_count = 36;
        
    };
}
