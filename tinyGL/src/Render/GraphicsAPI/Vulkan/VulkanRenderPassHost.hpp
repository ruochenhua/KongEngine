#pragma once

#ifdef RENDER_IN_VULKAN

#include "Render/Abstraction/IRenderPassHost.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkShadowMapRenderSystem.h"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkDeferRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkSimpleRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkSkyBoxRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkPostprocessRenderSystem.hpp"

namespace Kong
{
    class RenderModuleBackendVulkan;
    class VulkanDescriptorPool;

#define VK_RENDER_HOST_DEFER 1

    /** Vulkan：持有 Vk* 子系统并在 Descriptor 就绪后注册 Pass */
    class VulkanRenderPassHost final : public IRenderPassHost
    {
    public:
        VulkanRenderPassHost()  = default;
        ~VulkanRenderPassHost() override = default;

        void OnAttached(KongRenderModule& module, IGraphicsDevice* device) override;
        void RegisterDefaultRenderPasses(KongRenderModule& module) override;

        IRHIRenderSubsystem* TryGetRHISubsystem(RHISubsystemKind kind) override;
        void                 DrawFallbackVulkan(KongRenderModule& module, double delta) override;
        void AfterVulkanDescriptorReady(KongRenderModule& module, void* vkBackendOpaque) override;
        void OnVulkanReloadScene(KongRenderModule& module, void* descriptorPoolOpaque) override;

    private:
        void RegisterPassesInternal(KongRenderModule& module);

        std::unique_ptr<VkShadowMapRenderSystem>   m_vkShadowMapSystem;
        std::unique_ptr<VkDeferRenderSystem>       m_vkDeferRenderSystem;
        std::unique_ptr<SimpleVulkanRenderSystem>  m_vkSimpleRenderSystem;
        std::unique_ptr<VulkanSkyBoxRenderSystem>  m_vkSkyboxSystem;
        std::unique_ptr<VulkanPostprocessSystem>   m_vkPostProcessSystem;
    };
}

#endif
