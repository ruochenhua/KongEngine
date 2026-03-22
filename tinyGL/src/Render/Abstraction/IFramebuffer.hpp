/**
 * @file IFramebuffer.hpp
 * @brief RHI 帧缓冲：多颜色附件 + 可选深度模板，供 IRHICommandList::BindFramebuffer 使用。
 * @ingroup RenderAbstraction
 */

#pragma once

#include <cstdint>

namespace Kong
{
    class ITexture;

    /**
     * 与 API 无关的可渲染目标集合（MRT GBuffer、主场景 FBO 等）。
     * OpenGL：对应已完成的 FBO；Vulkan：对应 VkFramebuffer + 与之匹配的 RenderPass 起始信息（见 VulkanFramebufferRHI）。
     */
    class IFramebuffer
    {
    public:
        virtual ~IFramebuffer() = default;

        virtual int      GetWidth() const                = 0;
        virtual int      GetHeight() const               = 0;
        virtual uint32_t GetColorAttachmentCount() const = 0;

        /** index 越界时返回 nullptr */
        virtual ITexture* GetColorAttachment(int index = 0) = 0;
        /** 深度为 RBO 或非 ITexture 包装时可为 nullptr */
        virtual ITexture* GetDepthStencilAttachment() = 0;

        IFramebuffer(const IFramebuffer&) = delete;
        IFramebuffer& operator=(const IFramebuffer&) = delete;

    protected:
        IFramebuffer() = default;
    };
}
