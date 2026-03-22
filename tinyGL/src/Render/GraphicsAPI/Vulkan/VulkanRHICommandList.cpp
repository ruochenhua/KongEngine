#include "VulkanRHICommandList.hpp"

#ifdef RENDER_IN_VULKAN

#include "VkBufferRHI.hpp"
#include "Render/Abstraction/IFramebuffer.hpp"
#include "Render/Abstraction/ITexture.hpp"

namespace Kong
{
    void VulkanRHICommandList::Begin() {}

    void VulkanRHICommandList::End() {}

    void VulkanRHICommandList::SetViewport(float x, float y, float width, float height, float minDepth,
                                           float maxDepth)
    {
        (void)x;
        (void)y;
        (void)width;
        (void)height;
        (void)minDepth;
        (void)maxDepth;
    }

    void VulkanRHICommandList::SetScissor(int x, int y, int width, int height)
    {
        (void)x;
        (void)y;
        (void)width;
        (void)height;
    }

    void VulkanRHICommandList::BindFramebuffer(IFramebuffer* framebuffer)
    {
        (void)framebuffer;
    }

    void VulkanRHICommandList::ClearRenderTarget(RHIClearMask mask, const float* colorRGBA, float depth,
                                                 uint32_t stencil)
    {
        (void)mask;
        (void)colorRGBA;
        (void)depth;
        (void)stencil;
    }

    void VulkanRHICommandList::BindPipeline(IPipeline* pipeline)
    {
        (void)pipeline;
    }

    void VulkanRHICommandList::BindVertexBuffer(uint32_t slot, IBuffer* buffer, uint64_t offset)
    {
        if (m_cb == VK_NULL_HANDLE || !buffer) return;
        auto* vkBuf = dynamic_cast<VkBufferRHI*>(buffer);
        if (!vkBuf || !vkBuf->GetVulkanBuffer()) return;
        VkBuffer b = vkBuf->GetVulkanBuffer()->GetBuffer();
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
        if (m_cb == VK_NULL_HANDLE) return;
        vkCmdDraw(m_cb, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanRHICommandList::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount,
                                           uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                           uint32_t firstInstance)
    {
        (void)topology;
        if (m_cb == VK_NULL_HANDLE || !m_hasIndexBuffer) return;
        vkCmdDrawIndexed(m_cb, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanRHICommandList::BindTexture(uint32_t slot, ITexture* texture)
    {
        (void)slot;
        (void)texture;
    }

    void VulkanRHICommandList::BindUniformBuffer(uint32_t slot, IBuffer* buffer)
    {
        (void)slot;
        (void)buffer;
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
