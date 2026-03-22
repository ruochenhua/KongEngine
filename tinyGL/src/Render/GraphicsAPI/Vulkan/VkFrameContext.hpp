/**
 * @file VkFrameContext.hpp
 * @brief Vulkan IFrameContext：VkCommandBuffer + VulkanRHICommandList。
 */

#pragma once

#ifdef RENDER_IN_VULKAN
#include "Render/Abstraction/IFrameContext.hpp"
#include "VulkanRHICommandList.hpp"
#include <vulkan/vulkan_core.h>

namespace Kong
{
    class VkFrameContext : public IFrameContext
    {
    public:
        VkFrameContext() = default;
        void BeginFrame() override {}
        void EndFrame() override {}
        void* GetCurrentCommandList() override { return static_cast<void*>(m_commandBuffer); }

        void SetCommandBuffer(VkCommandBuffer cb)
        {
            m_commandBuffer = cb;
            m_rhiList.SetCommandBuffer(cb);
        }
        void SetFrameIndex(int index) { m_frameIndex = index; }
        int GetFrameIndex() const override { return m_frameIndex; }

        IRHICommandList* GetRHICommandList() override { return &m_rhiList; }

    private:
        VkCommandBuffer      m_commandBuffer {VK_NULL_HANDLE};
        int                  m_frameIndex {0};
        VulkanRHICommandList m_rhiList;
    };
}
#endif
