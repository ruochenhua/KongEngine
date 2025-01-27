#pragma once
#include <unordered_map>

#include "PostProcessRenderSystem.hpp"
#include "RenderSystem.hpp"
#include "Component/Mesh/QuadShape.h"
#include "Shader/DeferInfoShader.h"

namespace Kong
{
    class DeferRenderSystem : public KongRenderSystem
    {
    public:
        enum class DeferResType : uint8_t
        {
            Position    = 0,
            Normal,
            Albedo,
            Orm
        };

        void Init() override;

        RenderResultInfo Draw(double delta,
            const RenderResultInfo& render_result_info,
            KongRenderModule* render_module) override;

        GLuint GetPositionTexture();
        GLuint GetNormalTexture();
        GLuint GetAlbedoTexture();
        GLuint GetOrmTexture();

        GLuint GetFrameBuffer() const;

        shared_ptr<DeferredBRDFShader> GetDeferredBRDFShader() const {return m_deferredBRDFShader;}
    protected:
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
        
        GLuint m_renderToBuffer {0};    // 渲染到的buffer
        GLuint m_renderToRbo {0};
        GLuint m_renderToTextures[PP_TEXTURE_COUNT] = {0, 0, 0};
        
        void GenerateDeferInfoTextures(int width, int height);
        void GenerateDeferRenderToTextures(int width, int height);

        void RenderToBuffer(const KongRenderModule* render_module) const;
        void RenderToTexture(GLuint render_to_buffer, const KongRenderModule* render_module);

        shared_ptr<CQuadShape> m_quadShape;
    };
}
