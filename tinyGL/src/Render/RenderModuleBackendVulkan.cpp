/**
 * @file RenderModuleBackendVulkan.cpp
 * @brief Vulkan 后端：Descriptor/UBO 与各 Vk* 系统创建，以及 Pass 适配器注册。
 * 仅在 RENDER_IN_VULKAN 时参与编译。
 */

#ifdef RENDER_IN_VULKAN

#include "Render/RenderModuleBackendVulkan.hpp"
#include "Render/RenderModule.hpp"
#include "Render/Abstraction/RenderSystemAdapter.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Render/Abstraction/IGraphicsDevice.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanGraphicsDevice.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanDescriptor.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkShadowMapRenderSystem.h"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkDeferRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkSimpleRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkSkyBoxRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VkPostprocessRenderSystem.hpp"
#include "Render/RenderCommon.hpp"
#include "Component/LightComponent.h"
#include <memory>
#include <vulkan/vulkan_core.h>

#define VK_DEFER 1

namespace Kong
{
    RenderModuleBackendVulkan::RenderModuleBackendVulkan() = default;

    RenderModuleBackendVulkan::~RenderModuleBackendVulkan() = default;

    VkDescriptorSet RenderModuleBackendVulkan::GetDescriptorSet(uint32_t frameIndex) const
    {
        if (frameIndex < m_descriptorSets.size())
            return m_descriptorSets[frameIndex];
        return VK_NULL_HANDLE;
    }

    void RenderModuleBackendVulkan::Init(KongRenderModule* module, IGraphicsDevice* device)
    {
        (void)device;
        int meshCount = 30 * VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
        int meshTexCount = 30;
        m_descriptorPool = VulkanDescriptorPool::Builder()
            .SetMaxSets(meshCount)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshCount)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, meshCount)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, meshCount * meshTexCount)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, meshTexCount)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, meshTexCount)
            .Build();

        // 必须在创建 VkDeferRenderSystem / VulkanSkyBoxRenderSystem 等之前创建全局 descriptor layout，
        // 否则它们在 CreatePipelineLayout 时 GetDescriptorLayout() 为 null，pipeline layout 会得到 0 个 set 导致验证层报错。
        m_descriptorLayout = VulkanDescriptorSetLayout::Builder()
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .Build();
        m_descriptorLayout->m_usage = VulkanDescriptorSetLayout::GlobalData;
        m_uniformBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        m_descriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_uniformBuffers[i] = std::make_unique<VulkanBuffer>();
            m_uniformBuffers[i]->Initialize(UNIFORM_BUFFER, sizeof(KongRenderModule::GlobalVulkanUbo), 1);
            m_uniformBuffers[i]->Map();
            auto bufferInfo = m_uniformBuffers[i]->DescriptorInfo();
            VulkanDescriptorWriter(*m_descriptorLayout, *m_descriptorPool)
                .WriteBuffer(0, &bufferInfo)
                .Build(m_descriptorSets[i]);
        }

        VulkanShadowMapCreateInfo shadowMapCreateInfo{ m_descriptorPool.get() };
        m_vkShadowMapSystem = std::make_unique<VkShadowMapRenderSystem>(shadowMapCreateInfo);

        auto* vkDevice = VulkanGraphicsDevice::GetGraphicsDevice().get();
        auto* swapChain = vkDevice->GetSwapChain();

#if VK_DEFER
        m_vkDeferRenderSystem = std::make_unique<VkDeferRenderSystem>();
        m_vkDeferRenderSystem->CreateMeshDescriptorSet();
        VulkanSkyBoxRenderSystem::VulkanSkyBoxCreateInfo skyboxCreateInfo{
            m_descriptorPool.get(),
            m_vkDeferRenderSystem->GetColorTexture(),
            m_vkDeferRenderSystem->GetDepthTexture(),
        };
        m_vkSkyboxSystem = std::make_unique<VulkanSkyBoxRenderSystem>(skyboxCreateInfo);
        VulkanPostprocessSystem::VulkanPostprocessCreateInfo createInfo{
            swapChain, m_descriptorPool.get(),
            m_vkDeferRenderSystem->GetColorTexture()->m_imageView,
            m_vkDeferRenderSystem->GetColorTexture()->m_sampler,
            m_vkDeferRenderSystem->GetColorTexture()->m_image
        };
#else
        m_vkSimpleRenderSystem = std::make_unique<SimpleVulkanRenderSystem>();
        m_vkSimpleRenderSystem->CreateMeshDescriptorSet();
        VulkanSkyBoxRenderSystem::VulkanSkyBoxCreateInfo skyboxCreateInfo{
            m_descriptorPool.get(),
            m_vkSimpleRenderSystem->GetColorTexture(),
            m_vkSimpleRenderSystem->GetDepthTexture()
        };
        m_vkSkyboxSystem = std::make_unique<VulkanSkyBoxRenderSystem>(skyboxCreateInfo);
        VulkanPostprocessSystem::VulkanPostprocessCreateInfo createInfo{
            swapChain, m_descriptorPool.get(),
            m_vkSimpleRenderSystem->GetColorTexture()->m_imageView,
            m_vkSimpleRenderSystem->GetColorTexture()->m_sampler,
        };
#endif
        m_vkPostProcessSystem = std::make_unique<VulkanPostprocessSystem>(createInfo);

        RenderModuleBackendVulkan* self = this;
        module->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            self->m_vkShadowMapSystem->Draw(frameInfo);
        }));
#if VK_DEFER
        module->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            self->m_vkDeferRenderSystem->UpdateMeshUBO(frameInfo);
            self->m_vkDeferRenderSystem->Draw(frameInfo);
        }));
#else
        module->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            self->m_vkSimpleRenderSystem->UpdateMeshUBO(frameInfo);
            self->m_vkSimpleRenderSystem->Draw(frameInfo);
        }));
#endif
        module->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            self->m_vkSkyboxSystem->Draw(frameInfo);
        }));
        module->PushRenderSystem(std::make_unique<RenderSystemAdapter>([self](IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) {
            FrameInfo frameInfo{
                frameContext.GetFrameIndex(),
                sceneDrawInfo.frameTime,
                static_cast<VkCommandBuffer>(frameContext.GetCurrentCommandList())
            };
            self->m_vkPostProcessSystem->Draw(frameInfo);
        }));
    }

    void RenderModuleBackendVulkan::UpdateSceneRenderInfo(KongRenderModule* module)
    {
        shared_ptr<CCamera> cam = module->GetCamera();
        KongRenderModule::GlobalVulkanUbo ubo{};
        ubo.projection = cam->GetProjectionMatrix();
        ubo.view = cam->GetViewMatrix();
        ubo.cameraPosition = glm::vec4(cam->GetPosition(), 1.0f);
        // scene_render_info -> SceneLightInfo 在 UpdateSceneRenderInfo 已由 module 收集，这里需要从 module 取
        // 当前 KongRenderModule::UpdateSceneRenderInfo 先收集再写 UBO；我们改为 backend 只写 UBO，light_info 由 module 提供
        // 为简化，backend 从 module 的 scene_render_info 再填一次 SceneLightInfo（与 module 内逻辑一致）
        SSceneLightInfo& sri = module->scene_render_info;
        SceneLightInfo light_info;
        if (!sri.scene_dirlight.expired())
        {
            light_info.has_dir_light = glm::ivec4(1);
            auto dir_light = sri.scene_dirlight.lock();
            light_info.directional_light.light_dir = glm::vec4(dir_light->GetLightDir(), 1.0f);
            light_info.directional_light.light_color = glm::vec4(dir_light->light_color, 1.0f);
            light_info.directional_light.light_space_mat = dir_light->light_space_mat;
        }
        else
            light_info.has_dir_light = glm::ivec4(0);
        int point_light_count = 0;
        int point_light_shadow_count = 0;
        light_info.point_light_shadow_index = glm::ivec4(-1);
        for (const auto& light : sri.scene_pointlights)
        {
            if (point_light_count >= POINT_LIGHT_MAX) break;
            if (light.expired()) continue;
            auto point_light_ptr = light.lock();
            PointLight pl;
            pl.light_pos = glm::vec4(point_light_ptr->GetLightLocation(), 1.0f);
            pl.light_color = glm::vec4(point_light_ptr->light_color, 1.0f);
            light_info.point_lights[point_light_count] = pl;
            if (point_light_ptr->enable_shadowmap && point_light_shadow_count < POINT_LIGHT_SHADOW_MAX)
            {
                light_info.point_light_shadow_index[point_light_shadow_count] = point_light_count;
                ++point_light_shadow_count;
            }
            ++point_light_count;
        }
        light_info.point_light_count = glm::ivec4(point_light_count);
        ubo.sceneLightInfo = light_info;
        for (const auto& ub : m_uniformBuffers)
        {
            ub->WriteToBuffer(&ubo);
            ub->Flush();
        }
    }

    void RenderModuleBackendVulkan::OnReloadScene(KongRenderModule* module)
    {
        (void)module;
        m_vkShadowMapSystem->InitLightShadowMapResource(m_descriptorPool.get());
#if VK_DEFER
        m_vkDeferRenderSystem->CreateMeshDescriptorSet();
#else
        m_vkSimpleRenderSystem->CreateMeshDescriptorSet();
#endif
    }

    void RenderModuleBackendVulkan::DrawFallback(KongRenderModule* module, double delta)
    {
        (void)module;
        auto* vkDevice = VulkanGraphicsDevice::GetGraphicsDevice().get();
        VkCommandBuffer commandBuffer = vkDevice->GetCurrentCommandBuffer();
        if (!commandBuffer) return;
        int frameIndex = vkDevice->GetFrameIndex();
        FrameInfo frameInfo{ frameIndex, static_cast<float>(delta), commandBuffer };
        m_vkShadowMapSystem->Draw(frameInfo);
#if VK_DEFER
        m_vkDeferRenderSystem->UpdateMeshUBO(frameInfo);
        m_vkDeferRenderSystem->Draw(frameInfo);
#else
        m_vkSimpleRenderSystem->UpdateMeshUBO(frameInfo);
        m_vkSimpleRenderSystem->Draw(frameInfo);
#endif
        m_vkSkyboxSystem->Draw(frameInfo);
        m_vkPostProcessSystem->Draw(frameInfo);
    }
}

#endif
