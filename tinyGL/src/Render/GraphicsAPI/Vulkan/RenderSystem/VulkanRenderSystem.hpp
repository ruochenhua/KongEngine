#pragma once
#include <memory>
#include <vector>

#include "Render/GraphicsAPI/Vulkan/VulkanDescriptor.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanPipeline.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"

#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>

namespace Kong
{

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
    };

    class VulkanRenderSystem
    {
    public:
        VulkanRenderSystem(VulkanSwapChain* swapChain);
        virtual ~VulkanRenderSystem() = default;

        void BeginRenderPass(VkCommandBuffer commandBuffer);
        void EndRenderPass(VkCommandBuffer commandBuffer);
        
        VulkanRenderSystem(const VulkanRenderSystem&) = delete;
        VulkanRenderSystem& operator=(const VulkanRenderSystem&) = delete;
    
    protected:
        VulkanSwapChain* m_swapChain {nullptr};
        VkRenderPass m_renderPass {VK_NULL_HANDLE};
        VkFramebuffer m_framebuffer {VK_NULL_HANDLE};
        
        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout {VK_NULL_HANDLE};
        
        std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_descriptorSetLayout;
    };
}
#endif