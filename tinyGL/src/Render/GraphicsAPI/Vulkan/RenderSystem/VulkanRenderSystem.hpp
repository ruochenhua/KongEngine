#pragma once
#include <memory>
#include <vector>

#include "Render/GraphicsAPI/Vulkan/VulkanBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanDescriptor.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanPipeline.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"

#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>

namespace Kong
{
    class KongRenderModule;

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
    };

    class VulkanRenderSystem
    {
    public:
        VulkanRenderSystem();
        virtual ~VulkanRenderSystem() = default;

        void BeginRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer = VK_NULL_HANDLE);
        void EndRenderPass(VkCommandBuffer commandBuffer);
        
        VulkanRenderSystem(const VulkanRenderSystem&) = delete;
        VulkanRenderSystem& operator=(const VulkanRenderSystem&) = delete;
    
        template <class T>
        static std::vector<std::unique_ptr<VulkanBuffer>> CreateDescriptorBuffer();
    protected:
        VulkanSwapChain* m_swapChain {nullptr};
        VkRenderPass m_renderPass {VK_NULL_HANDLE};
        VkFramebuffer m_framebuffer {VK_NULL_HANDLE};
        
        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout {VK_NULL_HANDLE};
        
        std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_descriptorSetLayout;

        KongRenderModule* m_renderModule {nullptr};
        std::vector<VkClearValue> m_clearValues;
        
        VkExtent2D renderAreaExtent;
        // uniform buffer 
        std::vector<std::unique_ptr<VulkanBuffer>> m_uniformBuffers;

    };

    template <class T>
    std::vector<std::unique_ptr<VulkanBuffer>> VulkanRenderSystem::CreateDescriptorBuffer()
    {
        std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;
        uniformBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uniformBuffers.size(); ++i)
        {
            uniformBuffers[i] = std::make_unique<VulkanBuffer>();
            uniformBuffers[i]->Initialize(UNIFORM_BUFFER, sizeof(T), 1);
            uniformBuffers[i]->Map();
        }

        return std::move(uniformBuffers);
    }
}
#endif