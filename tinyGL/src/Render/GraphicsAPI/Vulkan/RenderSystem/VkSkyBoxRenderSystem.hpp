
#pragma once
#include "VulkanRenderSystem.hpp"
#include "Component/Mesh/BoxShape.h"
#include "Render/Resource/Texture.hpp"
#ifdef RENDER_IN_VULKAN

namespace Kong
{
    class VulkanSkyBoxRenderSystem : public VulkanRenderSystem
    {
    public:
        struct VulkanSkyBoxCreateInfo
        {
            VulkanDescriptorPool* descriptorPool {nullptr};
            VulkanTexture* inputSceneTexture {VK_NULL_HANDLE};
            VulkanTexture* inputDepthTexture {VK_NULL_HANDLE};
        };
    
        VulkanSkyBoxRenderSystem(const VulkanSkyBoxCreateInfo &createInfo);
        virtual ~VulkanSkyBoxRenderSystem();

        VulkanSkyBoxRenderSystem(const VulkanSkyBoxRenderSystem &) = delete;
        VulkanSkyBoxRenderSystem &operator=(const VulkanSkyBoxRenderSystem &) = delete;

        void Draw(const FrameInfo& frameInfo);
        
    private:
        void CreateDescriptorSetLayout();
        void CreatePipelineLayout();
        void CreatePipeline();
        void CreateRenderPass();
        void CreateCubeImage();
        void CreateDescriptorSet(const VulkanSkyBoxCreateInfo &createInfo);
        void CreateFramebuffer(const VulkanSkyBoxCreateInfo &createInfo);
        void SetBarrier(VkCommandBuffer commandBuffer);

        std::vector<
            std::map<VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType, VkDescriptorSet>
        > m_descriptorSets;
        
        std::unique_ptr<CBoxShape> m_boxShape {nullptr};
        shared_ptr<VulkanTexture> m_cubeMap;
        VulkanTexture* m_inputSceneTexture {nullptr};
        VulkanTexture* m_inputDepthTexture {nullptr};
        
        VkDescriptorImageInfo m_imageInfo;
    };
}

#endif
