#pragma once
#include <vulkan/vulkan_core.h>

#include "VulkanPipeline.hpp"

#ifdef RENDER_IN_VULKAN
namespace Kong
{
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet globalDescriptorSet;
    };
    
    class SimpleVulkanRenderSystem
    {
    public:
        SimpleVulkanRenderSystem(VulkanGraphicsDevice& deviceRef, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleVulkanRenderSystem();

        SimpleVulkanRenderSystem(const SimpleVulkanRenderSystem&) = delete;
        SimpleVulkanRenderSystem& operator=(const SimpleVulkanRenderSystem&) = delete;

        void RenderGameObjects(const FrameInfo& frameInfo);
    private:
        void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;

        
        VulkanGraphicsDevice& m_deviceRef;
    };
    
}


#endif