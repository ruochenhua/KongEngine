/**
 * @file RenderModuleBackendVulkan.hpp
 * @brief Vulkan 后端状态与 Pass 注册，仅于 RENDER_IN_VULKAN 构建时存在。
 */

#pragma once

#ifdef RENDER_IN_VULKAN

#include "Render/Abstraction/IRenderModuleBackend.hpp"
#include <memory>
#include <vector>
#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>
#endif

namespace Kong
{
    class VulkanDescriptorPool;
    class VulkanDescriptorSetLayout;
    class VulkanBuffer;
    class VkShadowMapRenderSystem;
    class VkDeferRenderSystem;
    class SimpleVulkanRenderSystem;
    class VulkanSkyBoxRenderSystem;
    class VulkanPostprocessSystem;

    class RenderModuleBackendVulkan : public IRenderModuleBackend
    {
    public:
        RenderModuleBackendVulkan();
        ~RenderModuleBackendVulkan() override;
        void Init(KongRenderModule* module, IGraphicsDevice* device) override;
        void UpdateSceneRenderInfo(KongRenderModule* module) override;
        void OnReloadScene(KongRenderModule* module) override;
        void DrawFallback(KongRenderModule* module, double delta) override;

        /** Vulkan 路径下供 VkDeferRenderSystem、LightComponent、MeshComponent 绑定全局 UBO 用 */
        VulkanDescriptorPool* GetDescriptorPool() const { return m_descriptorPool.get(); }
        VkDescriptorSet GetDescriptorSet(uint32_t frameIndex) const;
        /** 全局 UBO 的 descriptor set layout，供各 Vk* 系统创建 pipeline layout 用 */
        VulkanDescriptorSetLayout* GetDescriptorLayout() const { return m_descriptorLayout.get(); }

    private:
        std::unique_ptr<VulkanDescriptorPool> m_descriptorPool;
        std::unique_ptr<VulkanDescriptorSetLayout> m_descriptorLayout;
        std::vector<std::unique_ptr<VulkanBuffer>> m_uniformBuffers;
        std::vector<VkDescriptorSet> m_descriptorSets;
        std::unique_ptr<VkShadowMapRenderSystem> m_vkShadowMapSystem;
        std::unique_ptr<VkDeferRenderSystem> m_vkDeferRenderSystem;
        std::unique_ptr<SimpleVulkanRenderSystem> m_vkSimpleRenderSystem;
        std::unique_ptr<VulkanSkyBoxRenderSystem> m_vkSkyboxSystem;
        std::unique_ptr<VulkanPostprocessSystem> m_vkPostProcessSystem;
    };
}

#endif
