#include "VulkanGraphicsPipeline.hpp"

#ifdef RENDER_IN_VULKAN

#include "VkFrameContext.hpp"
#include "VulkanRHICommandList.hpp"

#include "Render/Abstraction/IFrameContext.hpp"
#include "Render/Abstraction/IRHICommandList.hpp"

namespace Kong
{
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkPipeline pipeline) : m_pipeline(pipeline) {}

    void VulkanGraphicsPipeline::Bind(IFrameContext& frameContext)
    {
        auto* vkCtx = dynamic_cast<VkFrameContext*>(&frameContext);
        if (vkCtx)
            BindGraphics(vkCtx->GetRHICommandList());
    }

    void VulkanGraphicsPipeline::BindGraphics(IRHICommandList* cmd)
    {
        auto* vkList = dynamic_cast<VulkanRHICommandList*>(cmd);
        if (!vkList)
            return;
        VkCommandBuffer cb = vkList->GetVulkanCommandBuffer();
        if (cb == VK_NULL_HANDLE || m_pipeline == VK_NULL_HANDLE)
            return;
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }
}

#endif
