#pragma once
#include "VulkanRenderSystem.hpp"
#ifdef RENDER_IN_VULKAN
namespace Kong
{
    class VulkanTexture;
    class VkDirectLightShadowMapRenderSystem;
    struct VulkanShadowMapCreateInfo
    {
        VulkanDescriptorPool* descriptorPool {nullptr};            
    };
    
    class VkShadowMapRenderSystem
    {
    public:
        VkShadowMapRenderSystem(const VulkanShadowMapCreateInfo &createInfo);
        virtual ~VkShadowMapRenderSystem();

        VkShadowMapRenderSystem(const VkShadowMapRenderSystem&) = delete;
        VkShadowMapRenderSystem& operator=(const VkShadowMapRenderSystem&) = delete;

        void InitLightShadowMapResource(VulkanDescriptorPool* descriptorPool);

        void Draw(const FrameInfo& frameInfo);

        VkImageView GetShadowMapDebugImageView();
        VkSampler GetShadowMapDebugSampler();

    private:
        // 平行光和点光源分开处理
        std::unique_ptr<VkDirectLightShadowMapRenderSystem> m_directLightShadowMapRenderSystem;
    };

    // 直接光照的阴影图
    class VkDirectLightShadowMapRenderSystem : public VulkanRenderSystem
    {
    public:
        VkDirectLightShadowMapRenderSystem(const VulkanShadowMapCreateInfo &createInfo);
        virtual ~VkDirectLightShadowMapRenderSystem();

        VkDirectLightShadowMapRenderSystem(const VkDirectLightShadowMapRenderSystem&) = delete;
        VkDirectLightShadowMapRenderSystem& operator=(const VkDirectLightShadowMapRenderSystem&) = delete;

        void Draw(const FrameInfo& frameInfo);
        // 初始化光源的阴影贴图资源
        void InitLightShadowMapResource(VulkanDescriptorPool* descriptorPool);

    private:
        void CreateRenderPass();
        void CreatePipeline();
        void CreatePipelineLayout();
        void CreateDescriptorSetLayout();

        // descriptor set
        std::vector<
            std::map<VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType, VkDescriptorSet>
        > m_descriptorSets;
    };
}

#endif