#pragma once
#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>

#include "VulkanPipeline.hpp"


namespace Kong
{
    class VulkanSwapChain;
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
        SimpleVulkanRenderSystem(VulkanSwapChain* swap_chain);
        ~SimpleVulkanRenderSystem();

        SimpleVulkanRenderSystem(const SimpleVulkanRenderSystem&) = delete;
        SimpleVulkanRenderSystem& operator=(const SimpleVulkanRenderSystem&) = delete;

        void UpdateMeshUBO(const FrameInfo& frameInfo);
        void CreateMeshDescriptorSet();
        void Draw(const FrameInfo& frameInfo, VkCommandBuffer commandBuffer);

        void BeginRenderPass(VkCommandBuffer commandBuffer);
        void EndRenderPass(VkCommandBuffer commandBuffer);

        VkImage GetColorImage() const { return m_image; }
        VkImageView GetColorImageView() const { return m_imageView; }
        VkSampler GetSampler() const {return m_sampler;}
    private:
        void CreateDescriptorSetLayout();
        void CreatePipelineLayout();
        void CreatePipeline();
        
        void CreateRenderPass();
        void CreateFrameBuffers();
        
        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        
        // 0=全局数据，1=基础材质数据，2=贴图
        std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_descriptorSetLayout;

        VulkanSwapChain* m_swapChain {nullptr};

        VkFramebuffer m_framebuffer;
        
        VkRenderPass m_renderPass {VK_NULL_HANDLE};
        
        VkImage m_image {VK_NULL_HANDLE};
        VkImageView m_imageView {VK_NULL_HANDLE};
        VkDeviceMemory m_imageMemory {VK_NULL_HANDLE};
        VkSampler m_sampler {VK_NULL_HANDLE};

        VkImage m_depthImage {VK_NULL_HANDLE};
        VkImageView m_depthImageView {VK_NULL_HANDLE};
        VkDeviceMemory m_depthImageMemory {VK_NULL_HANDLE};
    };
    
}


#endif