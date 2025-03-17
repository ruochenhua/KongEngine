#pragma once

#include "VkModelRenderSystem.hpp"
#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>


namespace Kong
{
    class VulkanPipeline;
    class VulkanSwapChain;
    class VulkanDescriptorPool;
    class VulkanBuffer;
    class VulkanDescriptorSetLayout;
    
    class SimpleVulkanRenderSystem : public VkModelRenderSystem
    {
    public:
        SimpleVulkanRenderSystem();
        ~SimpleVulkanRenderSystem() override;

        SimpleVulkanRenderSystem(const SimpleVulkanRenderSystem&) = delete;
        SimpleVulkanRenderSystem& operator=(const SimpleVulkanRenderSystem&) = delete;

        void Draw(const FrameInfo& frameInfo) override;
        
    private:
        // void CreateDescriptorSetLayout() override;
        // void CreatePipelineLayout() override;
        void CreatePipeline();
        void CreateRenderPass();
        void CreateFrameBuffers();
        
        // 0=全局数据，1=基础材质数据，2=贴图
        // std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_descriptorSetLayout;

    };
    
}


#endif