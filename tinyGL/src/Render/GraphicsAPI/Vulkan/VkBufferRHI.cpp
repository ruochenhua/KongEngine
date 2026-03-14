/**
 * @file VkBufferRHI.cpp
 * @brief Vulkan IBuffer 实现。
 * @ingroup RenderAbstraction
 */

#ifdef RENDER_IN_VULKAN
#include "VkBufferRHI.hpp"
#include "Render/Resource/Buffer.hpp"
#include <cstring>

namespace Kong
{
    static BufferType ToKongBufferType(BufferUsage usage)
    {
        switch (usage)
        {
        case BufferUsage::Vertex:  return VERTEX_BUFFER;
        case BufferUsage::Index:   return INDEX_BUFFER;
        case BufferUsage::Uniform: return UNIFORM_BUFFER;
        case BufferUsage::Staging: return VERTEX_BUFFER;
        default: return VERTEX_BUFFER;
        }
    }

    VkBufferRHI::VkBufferRHI(const BufferDesc& desc)
    {
        BufferType type = ToKongBufferType(desc.usage);
        uint64_t totalSize = 256u;
        if (desc.size > 0)
            totalSize = desc.size;
        else if (desc.instanceCount > 0)
            totalSize = static_cast<uint64_t>(desc.instanceCount) * 64u;

        m_impl = std::make_unique<VulkanBuffer>();
        m_impl->Initialize(type, totalSize, 1, desc.initialData);
    }

    VkBufferRHI::~VkBufferRHI() = default;

    void VkBufferRHI::Upload(const void* data, size_t size, size_t offset)
    {
        if (!data || !m_impl) return;
        VkDeviceSize vkSize = (size == 0) ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(size);
        m_impl->Map(vkSize, static_cast<VkDeviceSize>(offset));
        m_impl->WriteToBuffer(const_cast<void*>(data), vkSize, static_cast<VkDeviceSize>(offset));
        m_impl->Flush(vkSize, static_cast<VkDeviceSize>(offset));
        m_impl->Unmap();
    }

    void VkBufferRHI::Bind(uint32_t slot, void* commandList)
    {
        (void)slot;
        if (m_impl && commandList)
            m_impl->Bind(commandList);
    }
}
#endif
