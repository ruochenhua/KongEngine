/**
 * @file VkBufferRHI.hpp
 * @brief Vulkan 实现的 IBuffer，包装 VulkanBuffer。
 * @ingroup RenderAbstraction
 */

#pragma once

#ifdef RENDER_IN_VULKAN
#include "Render/Abstraction/IBuffer.hpp"
#include "Render/Abstraction/Types.hpp"
#include "VulkanBuffer.hpp"
#include <memory>

namespace Kong
{
    /** Vulkan 后端 IBuffer 实现，包装 VulkanBuffer */
    class VkBufferRHI : public IBuffer
    {
    public:
        explicit VkBufferRHI(const BufferDesc& desc);
        ~VkBufferRHI() override;

        void Upload(const void* data, size_t size = 0, size_t offset = 0) override;
        void Bind(uint32_t slot, void* commandList = nullptr) override;

        VulkanBuffer* GetVulkanBuffer() { return m_impl.get(); }
        const VulkanBuffer* GetVulkanBuffer() const { return m_impl.get(); }

    private:
        std::unique_ptr<VulkanBuffer> m_impl;
    };
}
#endif
