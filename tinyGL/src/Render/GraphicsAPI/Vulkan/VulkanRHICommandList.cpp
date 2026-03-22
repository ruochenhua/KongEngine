#include "VulkanRHICommandList.hpp"

#ifdef RENDER_IN_VULKAN

#include "VkBufferRHI.hpp"
#include "VulkanFramebufferRHI.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "Render/Abstraction/IFramebuffer.hpp"
#include "Render/Abstraction/IPipeline.hpp"
#include "Render/Abstraction/ITexture.hpp"

namespace Kong
{
    void VulkanRHICommandList::Begin()
    {
        m_inRenderPass = false;
        m_fbWidth = m_fbHeight = 0;
    }

    void VulkanRHICommandList::End()
    {
        EndRenderPass();
    }

    void VulkanRHICommandList::SetViewport(float x, float y, float width, float height, float minDepth,
                                           float maxDepth)
    {
        if (m_cb == VK_NULL_HANDLE)
            return;
        VkViewport vp{};
        vp.x         = x;
        vp.y         = y;
        vp.width     = width;
        vp.height    = height;
        vp.minDepth  = minDepth;
        vp.maxDepth  = maxDepth;
        vkCmdSetViewport(m_cb, 0, 1, &vp);
    }

    void VulkanRHICommandList::SetScissor(int x, int y, int width, int height)
    {
        if (m_cb == VK_NULL_HANDLE)
            return;
        VkRect2D sc{};
        sc.offset.x      = x;
        sc.offset.y      = y;
        sc.extent.width  = width > 0 ? static_cast<uint32_t>(width) : 0u;
        sc.extent.height = height > 0 ? static_cast<uint32_t>(height) : 0u;
        vkCmdSetScissor(m_cb, 0, 1, &sc);
    }

    void VulkanRHICommandList::BindFramebuffer(IFramebuffer* framebuffer)
    {
        if (m_cb == VK_NULL_HANDLE)
            return;

        if (m_inRenderPass)
        {
            vkCmdEndRenderPass(m_cb);
            m_inRenderPass = false;
        }

        m_fbWidth = m_fbHeight = 0;

        if (!framebuffer)
            return;

        auto* vkfb = dynamic_cast<VulkanFramebufferRHI*>(framebuffer);
        if (!vkfb || !vkfb->IsConfigured())
            return;

        const VkRenderPassBeginInfo* begin = vkfb->GetRenderPassBeginInfo();
        if (!begin)
            return;

        m_fbWidth  = vkfb->GetWidth();
        m_fbHeight = vkfb->GetHeight();

        vkCmdBeginRenderPass(m_cb, begin, VK_SUBPASS_CONTENTS_INLINE);
        m_inRenderPass = true;
    }

    void VulkanRHICommandList::EndRenderPass()
    {
        if (m_cb == VK_NULL_HANDLE || !m_inRenderPass)
            return;
        vkCmdEndRenderPass(m_cb);
        m_inRenderPass = false;
        m_fbWidth = m_fbHeight = 0;
    }

    void VulkanRHICommandList::ClearRenderTarget(RHIClearMask mask, const float* colorRGBA, float depth,
                                                 uint32_t stencil)
    {
        if (m_cb == VK_NULL_HANDLE || !m_inRenderPass || m_fbWidth <= 0 || m_fbHeight <= 0)
            return;

        VkClearRect clearRect{};
        clearRect.rect.offset           = {0, 0};
        clearRect.rect.extent.width   = static_cast<uint32_t>(m_fbWidth);
        clearRect.rect.extent.height  = static_cast<uint32_t>(m_fbHeight);
        clearRect.baseArrayLayer      = 0;
        clearRect.layerCount          = 1;

        VkClearAttachment attachments[2]{};
        uint32_t attCount = 0;

        if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(RHIClearMask::Color)) != 0)
        {
            VkClearAttachment& a = attachments[attCount++];
            a.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
            a.colorAttachment         = 0;
            a.clearValue.color.float32[0] = colorRGBA ? colorRGBA[0] : 0.f;
            a.clearValue.color.float32[1] = colorRGBA ? colorRGBA[1] : 0.f;
            a.clearValue.color.float32[2] = colorRGBA ? colorRGBA[2] : 0.f;
            a.clearValue.color.float32[3] = colorRGBA ? colorRGBA[3] : 1.f;
        }

        if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(RHIClearMask::Depth)) != 0
            || (static_cast<uint32_t>(mask) & static_cast<uint32_t>(RHIClearMask::Stencil)) != 0)
        {
            VkClearAttachment& a = attachments[attCount++];
            a.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(RHIClearMask::Stencil)) != 0)
                a.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            a.colorAttachment         = 0;
            a.clearValue.depthStencil.depth   = depth;
            a.clearValue.depthStencil.stencil = stencil;
        }

        if (attCount > 0)
            vkCmdClearAttachments(m_cb, attCount, attachments, 1, &clearRect);
    }

    void VulkanRHICommandList::BindPipeline(IPipeline* pipeline)
    {
        if (pipeline)
            pipeline->BindGraphics(this);
    }

    void VulkanRHICommandList::BindVertexBuffer(uint32_t slot, IBuffer* buffer, uint64_t offset)
    {
        if (m_cb == VK_NULL_HANDLE || !buffer)
            return;
        auto* vkBuf = dynamic_cast<VkBufferRHI*>(buffer);
        if (!vkBuf || !vkBuf->GetVulkanBuffer())
            return;
        VkBuffer     b   = vkBuf->GetVulkanBuffer()->GetBuffer();
        VkDeviceSize off = offset;
        vkCmdBindVertexBuffers(m_cb, slot, 1, &b, &off);
    }

    void VulkanRHICommandList::BindIndexBuffer(IBuffer* buffer, IndexElementType indexType, uint64_t offset)
    {
        if (m_cb == VK_NULL_HANDLE || !buffer)
        {
            m_hasIndexBuffer = false;
            return;
        }
        auto* vkBuf = dynamic_cast<VkBufferRHI*>(buffer);
        if (!vkBuf || !vkBuf->GetVulkanBuffer())
        {
            m_hasIndexBuffer = false;
            return;
        }
        vkCmdBindIndexBuffer(m_cb, vkBuf->GetVulkanBuffer()->GetBuffer(), offset, IndexTypeToVk(indexType));
        m_boundIndexType = indexType;
        m_hasIndexBuffer = true;
    }

    void VulkanRHICommandList::Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount,
                                    uint32_t firstVertex, uint32_t firstInstance)
    {
        (void)topology;
        if (m_cb == VK_NULL_HANDLE)
            return;
        vkCmdDraw(m_cb, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanRHICommandList::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount,
                                           uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                           uint32_t firstInstance)
    {
        (void)topology;
        if (m_cb == VK_NULL_HANDLE || !m_hasIndexBuffer)
            return;
        vkCmdDrawIndexed(m_cb, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanRHICommandList::BindTexture(uint32_t slot, ITexture* texture)
    {
        (void)slot;
        (void)texture;
    }

    void VulkanRHICommandList::BindUniformBuffer(uint32_t slot, IBuffer* buffer)
    {
        if (!buffer)
            return;
        buffer->Bind(slot, m_cb);
    }

    void VulkanRHICommandList::SetDepthWriteEnabled(bool enable)
    {
#if defined(VK_VERSION_1_3)
        if (m_cb != VK_NULL_HANDLE)
            vkCmdSetDepthWriteEnable(m_cb, enable ? VK_TRUE : VK_FALSE);
#else
        (void)enable;
#endif
    }

    void VulkanRHICommandList::SetDepthTestEnabled(bool enable)
    {
#if defined(VK_VERSION_1_3)
        if (m_cb != VK_NULL_HANDLE)
            vkCmdSetDepthTestEnable(m_cb, enable ? VK_TRUE : VK_FALSE);
#else
        (void)enable;
#endif
    }

    VkIndexType VulkanRHICommandList::IndexTypeToVk(IndexElementType t) const
    {
        switch (t)
        {
        case IndexElementType::UInt16: return VK_INDEX_TYPE_UINT16;
        case IndexElementType::UInt32:
        default: return VK_INDEX_TYPE_UINT32;
        }
    }
}

#endif
