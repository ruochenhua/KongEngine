#include "VulkanBuffer.hpp"
#ifdef RENDER_IN_VULKAN
using namespace Kong;

void VulkanBuffer::Initialize(BufferType type, uint64_t instanceSize, uint32_t instanceCount, void* data)
{
    KongBuffer::Initialize(type, instanceSize, instanceCount);
    
    m_instanceSize = instanceSize;
    m_instanceCount = instanceCount;

    switch (type)
    {
    case VERTEX_BUFFER:
        m_usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case INDEX_BUFFER:
        m_usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case UNIFORM_BUFFER:
        m_usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    default:
        throw std::runtime_error("VulkanBuffer::Initialize(): invalid buffer type");
        break;
    }

    // todo: 这里可能要再看下
    m_memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    m_alignmentSize = GetAlignment(m_instanceSize, 1);
    m_bufferSize = m_alignmentSize * m_instanceCount;
    
    auto device = VulkanGraphicsDevice::GetGraphicsDevice();
    device->CreateBuffer(m_bufferSize, m_usageFlags, m_memoryPropertyFlags, m_buffer, m_memory);

    // 有数据的话直接写入
    // todo: staging buffer 优化
    if (data)
    {
        Map();
        WriteToBuffer(data);
        Unmap();
    }
}

VulkanBuffer::~VulkanBuffer()
{
    Unmap();

    auto device = VulkanGraphicsDevice::GetGraphicsDevice();
    vkDestroyBuffer(device->GetDevice(), m_buffer, nullptr);
    vkFreeMemory(device->GetDevice(), m_memory, nullptr);
}


/**
 * 将一段内存和这个buffer映射，成功的话m_mapped指针将会指向对应的地址
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */

VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    assert(m_buffer && m_memory && "Called map on buffer before create");
    return vkMapMemory(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), m_memory, offset, size, 0, &m_mapped);
}

void VulkanBuffer::Unmap()
{
    if (m_mapped)
    {
        auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
        vkUnmapMemory(device, m_memory);
        m_mapped = nullptr;
    }
    
}

// 写入buffer
void VulkanBuffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
{
    assert(m_mapped && "Cannot copy to unmapped buffer");
    
    if (size == VK_WHOLE_SIZE)
    {
        // 覆盖整段内存
        memcpy(m_mapped, data, m_bufferSize);
    }
    else
    {
        char *memOffset = (char *)m_mapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

/**
 * 将CPU端的已映射的设备内存的修改同步到GPU端
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult VulkanBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), 1, &mappedRange);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo VulkanBuffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
    return VkDescriptorBufferInfo{
        m_buffer,
        offset,
        size,
    };
}

VkResult VulkanBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    /*
     * 在 Vulkan 里，当设备（GPU）对已映射到主机（CPU）的内存区域进行了修改后，主机端的缓存可能保存的是旧数据。
     * vkInvalidateMappedMemoryRanges 函数的作用就是使主机端对指定内存区域的缓存失效，从而让主机能够获取到设备端最新修改的数据。
     */
    return vkInvalidateMappedMemoryRanges(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), 1, &mappedRange);
}

void VulkanBuffer::WriteToIndex(void* data, int index)
{
    WriteToBuffer(data, m_bufferSize, index*m_alignmentSize);
}

VkResult VulkanBuffer::FlushIndex(int index)
{
    return Flush(m_alignmentSize, index*m_alignmentSize);
}

VkDescriptorBufferInfo VulkanBuffer::DescriptorInfoForIndex(int index)
{
    return DescriptorInfo(m_alignmentSize, index*m_alignmentSize);
}

VkResult VulkanBuffer::InvalidateIndex(int index)
{
    return Invalidate(m_alignmentSize, index*m_alignmentSize);
}

void VulkanBuffer::Bind(void* commandBuffer)
{
    auto cb = static_cast<VkCommandBuffer>(commandBuffer);
    switch (m_type)
    {
    case VERTEX_BUFFER:
        {
            VkBuffer buffers[] = {GetBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cb, 0, 1, buffers, offsets);
        }
        break;
    case INDEX_BUFFER:
        vkCmdBindIndexBuffer(cb, GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        break;
    default:
        break;
    }
}

VkDeviceSize VulkanBuffer::GetAlignment(VkDeviceSize size, VkDeviceSize minOffsetAlignment)
{
    if (minOffsetAlignment > 0) {
        return (size + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return size;
}

VulkanRenderInfo::VulkanRenderInfo()
{
    material = std::make_shared<RenderMaterialInfo>();
}

void VulkanRenderInfo::Draw(void* commandBuffer)
{
    auto cb = static_cast<VkCommandBuffer>(commandBuffer);

    if (!vertex_buffer->IsValid())
    {
        return;
    }
    
    vertex_buffer->Bind(commandBuffer);
    if (index_buffer && index_buffer->IsValid())
    {
        index_buffer->Bind(commandBuffer);
        
        vkCmdDrawIndexed(cb, static_cast<uint32_t>(m_Index.size()), 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(cb, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }
}

void VulkanRenderInfo::InitRenderInfo()
{
    vertex_buffer = make_unique<VulkanBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), vertices.size(), &vertices[0]);
    
    if(!m_Index.empty())
    {
        index_buffer = make_unique<VulkanBuffer>();
        index_buffer->Initialize(INDEX_BUFFER, sizeof(unsigned int), m_Index.size(), &m_Index[0]);
    }
}
#endif
