/**
 * @file IFramebuffer.hpp
 * @brief RHI 帧缓冲接口占位，阶段 5 实现。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    class ITexture;

    /** 与 API 无关的可渲染目标集合（颜色/深度等）；OpenGL 即 FBO，Vulkan 即 VkFramebuffer */
    class IFramebuffer
    {
    public:
        virtual ~IFramebuffer() = default;
        virtual ITexture* GetColorAttachment(int index = 0) = 0;
        virtual ITexture* GetDepthStencilAttachment() = 0;
        IFramebuffer(const IFramebuffer&) = delete;
        IFramebuffer& operator=(const IFramebuffer&) = delete;
    protected:
        IFramebuffer() = default;
    };
}
