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
        // VkDescriptorSet globalDescriptorSet;
    };
    
    class SimpleVulkanRenderSystem
    {
    public:
        struct SimpleVulkanUbo
        {
            glm::mat4 projectionView {1.f};
            glm::vec4 lightDirection = glm::normalize(glm::vec4{-1.f, -3.f, -1.0f, 0.0});
            glm::vec4 cameraPosition = glm::normalize(glm::vec4{1.f, 0.f, 0.f, 1.f});

            // SceneLightInfo sceneLightInfo;
            
        };
        
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

        
        std::unique_ptr<VulkanDescriptorPool> m_descriptorPool{};
        // 0=基础数据，1=贴图
        std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_descriptorSetLayout;

        //
        // std::vector<std::unique_ptr<VulkanBuffer>> m_uboBuffers;
        // std::vector<VkDescriptorSet> m_discriptorSets;
        
    };
    
}


#endif