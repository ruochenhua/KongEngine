#include "GLFramebuffer.hpp"
#include "Render/Abstraction/ITexture.hpp"

namespace Kong
{
    ITexture* GLFramebuffer::GetColorAttachment(int)
    {
        return nullptr;
    }

    ITexture* GLFramebuffer::GetDepthStencilAttachment()
    {
        return nullptr;
    }
}
