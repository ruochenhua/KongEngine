#pragma once

#ifdef RENDER_IN_VULKAN

#include "Render/Abstraction/IPipeline.hpp"
#include <vulkan/vulkan_core.h>

namespace Kong
{
    class VulkanGraphicsPipeline final : public IPipeline
    {
    public:
        explicit VulkanGraphicsPipeline(VkPipeline pipeline = VK_NULL_HANDLE);

        void SetPipeline(VkPipeline pipeline) { m_pipeline = pipeline; }
        VkPipeline GetVkPipeline() const { return m_pipeline; }

        void Bind(IFrameContext& frameContext) override;
        void BindGraphics(IRHICommandList* cmd) override;

    private:
        VkPipeline m_pipeline {VK_NULL_HANDLE};
    };
}

#endif
