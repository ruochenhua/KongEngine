#pragma once
#include "VulkanRenderSystem.hpp"

namespace Kong
{
    class VkDirectLightShadowMapRenderSystem;
    
    class VkShadowMapRenderSystem
    {
    public:
        VkShadowMapRenderSystem();
        virtual ~VkShadowMapRenderSystem();

        VkShadowMapRenderSystem(const VkShadowMapRenderSystem&) = delete;
        VkShadowMapRenderSystem& operator=(const VkShadowMapRenderSystem&) = delete;

        void Draw(const FrameInfo& frameInfo);

    private:
        // 平行光和点光源分开处理
        std::unique_ptr<VkDirectLightShadowMapRenderSystem> m_directLightShadowMapRenderSystem;
    };

    // 直接光照的阴影图
    class VkDirectLightShadowMapRenderSystem : public VulkanRenderSystem
    {
    public:
        VkDirectLightShadowMapRenderSystem() = default;
        virtual ~VkDirectLightShadowMapRenderSystem();

        VkDirectLightShadowMapRenderSystem(const VkDirectLightShadowMapRenderSystem&) = delete;
        VkDirectLightShadowMapRenderSystem& operator=(const VkDirectLightShadowMapRenderSystem&) = delete;

        void Draw(const FrameInfo& frameInfo);

    private:
        void CreateRenderPass();
        void CreateDescriptorSetLayout();
        void CreatePipeline();
        void CreatePipelineLayout();
        void CreateDescriptorSet();
        void CreateFramebuffer();

        
    };
}