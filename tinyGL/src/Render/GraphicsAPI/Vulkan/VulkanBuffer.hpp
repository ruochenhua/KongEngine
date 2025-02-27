#pragma once
#include "VulkanGraphicsDevice.hpp"
#include "Render/RenderCommon.hpp"
#include "Render/Resource/Buffer.hpp"
#ifdef RENDER_IN_VULKAN
namespace Kong
{
    class VulkanBuffer : public KongBuffer
    {
    public:
        // instanceSize代表数据类型的大小sizeof(vertex)
        void Initialize(BufferType type, uint64_t instanceSize, uint32_t instanceCount, void* data = nullptr) override;
        
        ~VulkanBuffer() override;

        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();

        void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void WriteToIndex(void* data, int index);
        VkResult FlushIndex(int index);
        VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
        VkResult InvalidateIndex(int index);
        
        VkBuffer GetBuffer() const { return m_buffer; }
        void* GetMappedMemory() const { return m_mapped; }
        uint32_t GetInstanceCount() const { return m_instanceCount; }
        VkDeviceSize GetInstanceSize() const { return m_instanceSize; }
        VkDeviceSize GetAlignmentSize() const { return m_instanceSize; }
        VkBufferUsageFlags GetUsageFlags() const { return m_usageFlags; }
        VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return m_memoryPropertyFlags; }
        VkDeviceSize GetBufferSize() const { return m_bufferSize; }

        void Bind(void* commandBuffer) override;
    private:
        static VkDeviceSize GetAlignment(VkDeviceSize size, VkDeviceSize minOffsetAlignment);
        
        void* m_mapped = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;

        VkDeviceSize m_bufferSize = 0;
        uint32_t m_instanceCount = 0;
        VkDeviceSize m_instanceSize = 0;
        VkDeviceSize m_alignmentSize = 0;
        VkBufferUsageFlags m_usageFlags = 0;
        VkMemoryPropertyFlags m_memoryPropertyFlags = 0;
    };

}
#endif