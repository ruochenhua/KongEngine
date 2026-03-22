#pragma once

#include "Render/Abstraction/IFramebuffer.hpp"
#include "glad/glad.h"

namespace Kong
{
    class GLFramebuffer : public IFramebuffer
    {
    public:
        explicit GLFramebuffer(GLuint fbo = 0) : m_fbo(fbo) {}

        ITexture* GetColorAttachment(int index = 0) override;
        ITexture* GetDepthStencilAttachment() override;

        GLuint GetGLName() const { return m_fbo; }
        void   SetGLName(GLuint fbo) { m_fbo = fbo; }

    private:
        GLuint m_fbo {0};
    };
}
