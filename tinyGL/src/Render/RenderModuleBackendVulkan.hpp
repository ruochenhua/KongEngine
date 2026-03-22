/**
 * @file RenderModuleBackendVulkan.hpp
 * @brief Vulkan：仅全局 Descriptor Pool / Set Layout / 每帧 UBO；Vk* 子系统在 KongRenderModule。
 */

#pragma once

#ifdef RENDER_IN_VULKAN

#include "Render/Abstraction/IRenderModuleBackend.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Kong
{
    class VulkanDescriptorPool;
    class VulkanDescriptorSetLayout;
    class VulkanBuffer;

    class RenderModuleBackendVulkan : public IRenderModuleBackend
    {
    public:
        RenderModuleBackendVulkan();
        ~RenderModuleBackendVulkan() override;
        void Init(KongRenderModule* renderModule, IGraphicsDevice* device) override;
        void UpdateSceneRenderInfo(KongRenderModule* renderModule) override;
        void OnReloadScene(KongRenderModule* renderModule) override;
        void DrawFallback(KongRenderModule* renderModule, double delta) override;

        VulkanDescriptorPool*       GetDescriptorPool() const { return m_descriptorPool.get(); }
        VkDescriptorSet             GetDescriptorSet(uint32_t frameIndex) const;
        VulkanDescriptorSetLayout*  GetDescriptorLayout() const { return m_descriptorLayout.get(); }

    private:
        std::unique_ptr<VulkanDescriptorPool>       m_descriptorPool;
        std::unique_ptr<VulkanDescriptorSetLayout>  m_descriptorLayout;
        std::vector<std::unique_ptr<VulkanBuffer>>  m_uniformBuffers;
        std::vector<VkDescriptorSet>              m_descriptorSets;
    };
}

#endif
