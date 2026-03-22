#include "GLFramebuffer.hpp"
#include "GLTexture.hpp"

#include "Render/Abstraction/ITexture.hpp"

namespace Kong
{
    GLFramebuffer::GLFramebuffer() = default;

    GLFramebuffer::~GLFramebuffer()
    {
        DestroyOwnedFbo();
    }

    void GLFramebuffer::DestroyOwnedFbo()
    {
        if (m_ownsFbo && m_fbo)
        {
            glDeleteFramebuffers(1, &m_fbo);
            m_fbo = 0;
        }
        m_ownsFbo = false;
    }

    void GLFramebuffer::SetGLName(GLuint fbo, bool takeOwnership)
    {
        DestroyOwnedFbo();
        m_fbo       = fbo;
        m_ownsFbo   = takeOwnership;
        m_colorViews.clear();
        m_colorRawPtrs.clear();
        m_depthView.reset();
    }

    void GLFramebuffer::SetFromGLAttachments(GLuint fbo, int width, int height, const GLuint* colorTextureIds,
                                             uint32_t colorCount, GLuint depthStencilTextureId)
    {
        DestroyOwnedFbo();
        m_fbo     = fbo;
        m_ownsFbo = false;
        m_width   = width;
        m_height  = height;

        m_colorViews.clear();
        m_colorRawPtrs.clear();
        m_depthView.reset();

        if (colorTextureIds && colorCount > 0)
        {
            m_colorViews.reserve(colorCount);
            m_colorRawPtrs.reserve(colorCount);
            for (uint32_t i = 0; i < colorCount; ++i)
            {
                m_colorViews.push_back(std::make_unique<GLTextureView>(colorTextureIds[i], width, height));
                m_colorRawPtrs.push_back(m_colorViews.back().get());
            }
        }

        if (depthStencilTextureId != 0)
            m_depthView = std::make_unique<GLTextureView>(depthStencilTextureId, width, height);
    }

    ITexture* GLFramebuffer::GetColorAttachment(int index)
    {
        if (index < 0 || static_cast<size_t>(index) >= m_colorRawPtrs.size())
            return nullptr;
        return m_colorRawPtrs[static_cast<size_t>(index)];
    }

    void GLFramebuffer::ApplyDrawBuffers() const
    {
        const uint32_t n = GetColorAttachmentCount();
        if (n == 0)
        {
            glDrawBuffer(GL_NONE);
            return;
        }
        if (n > 8)
            return;
        GLenum bufs[8];
        for (uint32_t i = 0; i < n; ++i)
            bufs[i] = GL_COLOR_ATTACHMENT0 + i;
        glDrawBuffers(static_cast<GLsizei>(n), bufs);
    }
}
