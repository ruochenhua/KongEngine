#pragma once

#ifdef RENDER_IN_VULKAN

#include "Render/Abstraction/IFramebuffer.hpp"
#include <vulkan/vulkan_core.h>

#include <vector>

namespace Kong
{
    /**
     * Vulkan 离屏目标：封装 VkRenderPassBeginInfo 所需字段，供 VulkanRHICommandList::BindFramebuffer 调用 vkCmdBeginRenderPass。
     * 颜色附件的 ITexture* 查询暂未桥接（返回 nullptr）；尺寸与附件数由 Configure 显式提供。
     */
    class VulkanFramebufferRHI final : public IFramebuffer
    {
    public:
        VulkanFramebufferRHI() = default;

        int      GetWidth() const override { return m_width; }
        int      GetHeight() const override { return m_height; }
        uint32_t GetColorAttachmentCount() const override { return m_colorAttachmentCount; }

        ITexture* GetColorAttachment(int /*index*/) override { return nullptr; }
        ITexture* GetDepthStencilAttachment() override { return nullptr; }

        /**
         * @param colorAttachmentCount 逻辑颜色附件数量（用于 GetColorAttachmentCount）；可与 clearValues 数量一致。
         */
        void Configure(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea,
                       std::vector<VkClearValue> clearValues, int width, int height,
                       uint32_t colorAttachmentCount);

        bool IsConfigured() const { return m_valid; }

        /** 供 VulkanRHICommandList 使用；指针在对象生命周期内有效 */
        const VkRenderPassBeginInfo* GetRenderPassBeginInfo() const;

    private:
        bool m_valid {false};
        int  m_width {0};
        int  m_height {0};
        uint32_t m_colorAttachmentCount {0};

        VkRenderPass   m_renderPass {VK_NULL_HANDLE};
        VkFramebuffer  m_framebuffer {VK_NULL_HANDLE};
        VkRect2D       m_renderArea {};

        std::vector<VkClearValue> m_clearValues;

        mutable VkRenderPassBeginInfo m_cachedBegin {};
    };
}

#endif
