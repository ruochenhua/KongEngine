#pragma once

#include "GLTexture.hpp"
#include "Render/Abstraction/IFramebuffer.hpp"
#include "glad/glad.h"

#include <memory>
#include <vector>

namespace Kong
{

    /**
     * OpenGL FBO 的 RHI 封装：支持 MRT、查询附件 ITexture*（GLTextureView）、Bind 时设置 glDrawBuffers。
     */
    class GLFramebuffer : public IFramebuffer
    {
    public:
        GLFramebuffer();
        ~GLFramebuffer() override;

        int      GetWidth() const override { return m_width; }
        int      GetHeight() const override { return m_height; }
        uint32_t GetColorAttachmentCount() const override
        {
            return static_cast<uint32_t>(m_colorViews.size());
        }

        ITexture* GetColorAttachment(int index = 0) override;
        ITexture* GetDepthStencilAttachment() override { return m_depthView.get(); }

        GLuint GetGLName() const { return m_fbo; }
        void   SetGLName(GLuint fbo, bool takeOwnership = false);

        /**
         * 使用已有 FBO 与颜色纹理 ID（如主场景 MRT）；深度为 Renderbuffer 时 depthStencil 可为 0。
         * 为每个颜色附件创建 GLTextureView（托管在 m_colorViews）。
         */
        void SetFromGLAttachments(GLuint fbo, int width, int height, const GLuint* colorTextureIds,
                                  uint32_t colorCount, GLuint depthStencilTextureId = 0);

        /** 绑定 FBO 后调用：按颜色附件数设置 glDrawBuffers(GL_COLOR_ATTACHMENT0+i) */
        void ApplyDrawBuffers() const;

    private:
        void DestroyOwnedFbo();

        GLuint m_fbo {0};
        bool   m_ownsFbo {false};
        int    m_width {0};
        int    m_height {0};

        std::vector<std::unique_ptr<GLTextureView>> m_colorViews;
        std::vector<ITexture*>                      m_colorRawPtrs;
        std::unique_ptr<GLTextureView>              m_depthView;
    };
}
