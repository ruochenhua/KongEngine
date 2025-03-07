
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
            VulkanSwapChain* swapChain {nullptr};
            VkFramebuffer frameBuffer {VK_NULL_HANDLE};
            VulkanDescriptorPool* descriptorPool {nullptr};
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

        std::vector<
            std::map<VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType, VkDescriptorSet>
        > m_descriptorSets;
        
        std::unique_ptr<CBoxShape> m_boxShape {nullptr};
        // todo: 放这里面
        VulkanTexture m_cubeMap;
        VkImage m_image;
        VkImageView m_imageView;
        VkSampler m_sampler;
        VkDeviceMemory m_imageMemory;
        
        VkDescriptorImageInfo m_imageInfo {VK_NULL_HANDLE};
    };
}

#endif
