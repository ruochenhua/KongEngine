/**
 * @file VkFrameContext.hpp
 * @brief Vulkan 实现的 IFrameContext，包装当前帧的 VkCommandBuffer。
 * @ingroup RenderAbstraction
 */

#pragma once

#ifdef RENDER_IN_VULKAN
#include "Render/Abstraction/IFrameContext.hpp"
#include <vulkan/vulkan_core.h>

namespace Kong
{
    /** Vulkan 后端 IFrameContext：持有当前帧的 command buffer，BeginFrame/EndFrame 由设备或 Module 驱动 */
    class VkFrameContext : public IFrameContext
    {
    public:
        VkFrameContext() = default;
        void BeginFrame() override {}
        void EndFrame() override {}
        void* GetCurrentCommandList() override { return static_cast<void*>(m_commandBuffer); }

        void SetCommandBuffer(VkCommandBuffer cb) { m_commandBuffer = cb; }
        void SetFrameIndex(int index) { m_frameIndex = index; }
        int GetFrameIndex() const override { return m_frameIndex; }

    private:
        VkCommandBuffer m_commandBuffer {VK_NULL_HANDLE};
        int             m_frameIndex {0};
    };
}
#endif
