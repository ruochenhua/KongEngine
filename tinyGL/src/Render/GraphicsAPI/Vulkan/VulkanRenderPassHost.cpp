/**
 * @file VulkanRenderPassHost.cpp
 */

#ifdef RENDER_IN_VULKAN

#include "VulkanRenderPassHost.hpp"

#include "Render/GraphicsAPI/Vulkan/VulkanDescriptor.hpp"
#include "Render/RenderModule.hpp"
#include "Render/RenderModuleBackendVulkan.hpp"
#include "Render/Abstraction/IFrameContext.hpp"
#include "Render/Abstraction/RenderSystemAdapter.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanGraphicsDevice.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"
#include "Render/RenderCommon.hpp"
#include <vulkan/vulkan_core.h>

namespace Kong
{
    void VulkanRenderPassHost::OnAttached(KongRenderModule& module, IGraphicsDevice* device)
    {
        (void)module;
        (void)device;
    }

    void VulkanRenderPassHost::RegisterDefaultRenderPasses(KongRenderModule& module)
    {
        (void)module;
    }

    void VulkanRenderPassHost::AfterVulkanDescriptorReady(KongRenderModule& module, void* vkBackendOpaque)
    {
        auto* vkBackend = static_cast<RenderModuleBackendVulkan*>(vkBackendOpaque);
        VulkanShadowMapCreateInfo shadowMapCreateInfo{ vkBackend->GetDescriptorPool() };
        m_vkShadowMapSystem = std::make_unique<VkShadowMapRenderSystem>(shadowMapCreateInfo);

        auto* vkDevice = VulkanGraphicsDevice::GetGraphicsDevice().get();
        auto* swapChain = vkDevice->GetSwapChain();

#if VK_RENDER_HOST_DEFER
        m_vkDeferRenderSystem = std::make_unique<VkDeferRenderSystem>();
        m_vkDeferRenderSystem->CreateMeshDescriptorSet();
        VulkanSkyBoxRenderSystem::VulkanSkyBoxCreateInfo skyboxCreateInfo{
            vkBackend->GetDescriptorPool(),
            m_vkDeferRenderSystem->GetColorTexture(),
            m_vkDeferRenderSystem->GetDepthTexture(),
        };
        m_vkSkyboxSystem = std::make_unique<VulkanSkyBoxRenderSystem>(skyboxCreateInfo);
        VulkanPostprocessSystem::VulkanPostprocessCreateInfo createInfo{
            swapChain, vkBackend->GetDescriptorPool(),
            m_vkDeferRenderSystem->GetColorTexture()->m_imageView,
            m_vkDeferRenderSystem->GetColorTexture()->m_sampler,
            m_vkDeferRenderSystem->GetColorTexture()->m_image
        };
#else
        m_vkSimpleRenderSystem = std::make_unique<SimpleVulkanRenderSystem>();
        m_vkSimpleRenderSystem->CreateMeshDescriptorSet();
        VulkanSkyBoxRenderSystem::VulkanSkyBoxCreateInfo skyboxCreateInfo{
            vkBackend->GetDescriptorPool(),
            m_vkSimpleRenderSystem->GetColorTexture(),
            m_vkSimpleRenderSystem->GetDepthTexture()
        };
        m_vkSkyboxSystem = std::make_unique<VulkanSkyBoxRenderSystem>(skyboxCreateInfo);
        VulkanPostprocessSystem::VulkanPostprocessCreateInfo createInfo{
            swapChain, vkBackend->GetDescriptorPool(),
            m_vkSimpleRenderSystem->GetColorTexture()->m_imageView,
            m_vkSimpleRenderSystem->GetColorTexture()->m_sampler,
            m_vkSimpleRenderSystem->GetColorTexture()->m_image,
        };
#endif
        m_vkPostProcessSystem = std::make_unique<VulkanPostprocessSystem>(createInfo);

        RegisterPassesInternal(module);
    }

    void VulkanRenderPassHost::RegisterPassesInternal(KongRenderModule& module)
    {
        KongRenderModule*     rm = &module;
        VulkanRenderPassHost* h  = this;
        module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([rm, h](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            h->m_vkShadowMapSystem->Draw(frameInfo);
        }));
#if VK_RENDER_HOST_DEFER
        module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([rm, h](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            (void)rm;
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            h->m_vkDeferRenderSystem->UpdateMeshUBO(frameInfo);
            h->m_vkDeferRenderSystem->Draw(frameInfo);
        }));
#else
        module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([rm, h](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            (void)rm;
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            h->m_vkSimpleRenderSystem->UpdateMeshUBO(frameInfo);
            h->m_vkSimpleRenderSystem->Draw(frameInfo);
        }));
#endif
        module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([rm, h](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            (void)rm;
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            h->m_vkSkyboxSystem->Draw(frameInfo);
        }));
        module.PushRenderSystem(std::make_unique<RenderSystemAdapter>([rm, h](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            (void)rm;
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            h->m_vkPostProcessSystem->Draw(frameInfo);
        }));
    }

    IRHIRenderSubsystem* VulkanRenderPassHost::TryGetRHISubsystem(RHISubsystemKind kind)
    {
        switch (kind)
        {
        case RHISubsystemKind::Deferred:
#if VK_RENDER_HOST_DEFER
            return m_vkDeferRenderSystem.get();
#else
            return m_vkSimpleRenderSystem.get();
#endif
        case RHISubsystemKind::Skybox: return m_vkSkyboxSystem.get();
        case RHISubsystemKind::PostProcess: return m_vkPostProcessSystem.get();
        default: return nullptr;
        }
    }

    void VulkanRenderPassHost::DrawFallbackVulkan(KongRenderModule& module, double delta)
    {
        auto* vkDevice = VulkanGraphicsDevice::GetGraphicsDevice().get();
        VkCommandBuffer commandBuffer = vkDevice->GetCurrentCommandBuffer();
        if (!commandBuffer) return;
        int       frameIndex = vkDevice->GetFrameIndex();
        FrameInfo frameInfo{frameIndex, static_cast<float>(delta), commandBuffer};
        m_vkShadowMapSystem->Draw(frameInfo);
#if VK_RENDER_HOST_DEFER
        m_vkDeferRenderSystem->UpdateMeshUBO(frameInfo);
        m_vkDeferRenderSystem->Draw(frameInfo);
#else
        m_vkSimpleRenderSystem->UpdateMeshUBO(frameInfo);
        m_vkSimpleRenderSystem->Draw(frameInfo);
#endif
        m_vkSkyboxSystem->Draw(frameInfo);
        m_vkPostProcessSystem->Draw(frameInfo);
        (void)module;
    }

    void VulkanRenderPassHost::OnVulkanReloadScene(KongRenderModule& module, void* descriptorPoolOpaque)
    {
        auto* pool = static_cast<VulkanDescriptorPool*>(descriptorPoolOpaque);
        m_vkShadowMapSystem->InitLightShadowMapResource(pool);
#if VK_RENDER_HOST_DEFER
        m_vkDeferRenderSystem->CreateMeshDescriptorSet();
#else
        m_vkSimpleRenderSystem->CreateMeshDescriptorSet();
#endif
        (void)module;
    }
}

#endif
