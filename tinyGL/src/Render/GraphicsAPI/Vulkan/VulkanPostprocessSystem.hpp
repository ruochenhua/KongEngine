#pragma once
#include "Render/RenderSystem/DeferRenderSystem.hpp"
#ifdef RENDER_IN_VULKAN

namespace Kong
{
    class VulkanSwapChain;

    struct VulkanPostprocessUbo
    {
        float exposure {0.1f};
        int bloom {0};
    };
    
    
    class VulkanPostprocessSystem
    {
    public:
        VulkanPostprocessSystem(VulkanSwapChain* swapChain, VulkanDescriptorPool* descriptorPool);
        ~VulkanPostprocessSystem();

        void Draw(const FrameInfo& frameInfo);
        VulkanPostprocessSystem(const VulkanPostprocessSystem&) = delete;
        VulkanPostprocessSystem& operator=(const VulkanPostprocessSystem&) = delete;
    private:
        // VkRenderPass m_renderPass {VK_NULL_HANDLE};

        // todo: 这个流程可能每个vulkan render system都会有，考虑流程化
        void CreateDescriptorSetLayout();
        void CreatePipelineLayout();
        void CreatePipeline();
        void CreateRenderPass();
        void CreateDescriptorBuffer();
        // 后处理的framebuffer应该就是swapchain的framebuffers
        // todo: 感觉frame buffers应该还是移到这里比较好
        void CreateFrameBuffers() {}

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout {VK_NULL_HANDLE};

        // scene/bright texture
        std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_descriptorSetLayout;

        VulkanSwapChain* m_swapChain {nullptr};
        
        std::vector<std::map<VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType, VkDescriptorSet>> m_descriptorSets;
        
        std::unique_ptr<CQuadShape> quadShape {nullptr};

        VkDescriptorImageInfo m_imageInfo {VK_NULL_HANDLE};

        std::vector<std::unique_ptr<VulkanBuffer>> m_uniformBuffers;
    };
}
#endif