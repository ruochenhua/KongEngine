#include "VulkanFramebufferRHI.hpp"

#ifdef RENDER_IN_VULKAN

namespace Kong
{
    void VulkanFramebufferRHI::Configure(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea,
                                          std::vector<VkClearValue> clearValues, int width, int height,
                                          uint32_t colorAttachmentCount)
    {
        m_renderPass           = renderPass;
        m_framebuffer          = framebuffer;
        m_renderArea           = renderArea;
        m_clearValues          = std::move(clearValues);
        m_width                = width;
        m_height               = height;
        m_colorAttachmentCount = colorAttachmentCount;
        m_valid                = (m_renderPass != VK_NULL_HANDLE && m_framebuffer != VK_NULL_HANDLE && m_width > 0
                   && m_height > 0);
    }

    const VkRenderPassBeginInfo* VulkanFramebufferRHI::GetRenderPassBeginInfo() const
    {
        if (!m_valid)
            return nullptr;

        m_cachedBegin                 = {};
        m_cachedBegin.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        m_cachedBegin.renderPass      = m_renderPass;
        m_cachedBegin.framebuffer     = m_framebuffer;
        m_cachedBegin.renderArea      = m_renderArea;
        m_cachedBegin.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
        m_cachedBegin.pClearValues    = m_clearValues.empty() ? nullptr : m_clearValues.data();
        return &m_cachedBegin;
    }
}

#endif
