#pragma once
#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>

#include "VulkanPipeline.hpp"


namespace Kong
{
    class VulkanDescriptorPool;
    class VulkanBuffer;
    class VulkanDescriptorSetLayout;

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
    };
    
    class SimpleVulkanRenderSystem
    {
    public:
        SimpleVulkanRenderSystem(VkRenderPass renderPass);
        ~SimpleVulkanRenderSystem();

        SimpleVulkanRenderSystem(const SimpleVulkanRenderSystem&) = delete;
        SimpleVulkanRenderSystem& operator=(const SimpleVulkanRenderSystem&) = delete;

        void UpdateMeshUBO(const FrameInfo& frameInfo);
        void CreateMeshDescriptorSet();
        void RenderGameObjects(const FrameInfo& frameInfo);
    private:
        void CreateDescriptorSetLayout();
        void CreatePipelineLayout();
        void CreatePipeline(VkRenderPass renderPass);

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        
        shared_ptr<VulkanGraphicsDevice> m_deviceRef;
        
        // 0=基础数据，1=贴图
        std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_descriptorSetLayout;
    };
    
}


#endif