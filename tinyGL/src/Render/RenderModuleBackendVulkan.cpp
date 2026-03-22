/**
 * @file RenderModuleBackendVulkan.cpp
 * @brief Vulkan：Descriptor / 全局 UBO；Pass 与子系统在 IRenderPassHost::AfterVulkanDescriptorReady。
 */

#ifdef RENDER_IN_VULKAN

#include "Render/RenderModuleBackendVulkan.hpp"
#include "Render/RenderModule.hpp"
#include "Render/Abstraction/IGraphicsDevice.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanDescriptor.hpp"
#include "Render/RenderCommon.hpp"
#include "Component/CameraComponent.h"
#include <memory>

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

    void RenderModuleBackendVulkan::Init(KongRenderModule* renderModule, IGraphicsDevice* device)
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

        if (auto* host = renderModule->GetRenderPassHost())
            host->AfterVulkanDescriptorReady(*renderModule, this);
    }

    void RenderModuleBackendVulkan::UpdateSceneRenderInfo(KongRenderModule* renderModule)
    {
        shared_ptr<CCamera> cam = renderModule->GetCamera();
        KongRenderModule::GlobalVulkanUbo ubo{};
        ubo.projection = cam->GetProjectionMatrix();
        ubo.view = cam->GetViewMatrix();
        ubo.cameraPosition = glm::vec4(cam->GetPosition(), 1.0f);
        SSceneLightInfo& sri = renderModule->scene_render_info;
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

    void RenderModuleBackendVulkan::OnReloadScene(KongRenderModule* renderModule)
    {
        if (auto* host = renderModule->GetRenderPassHost())
            host->OnVulkanReloadScene(*renderModule, m_descriptorPool.get());
    }

    void RenderModuleBackendVulkan::DrawFallback(KongRenderModule* renderModule, double delta)
    {
        if (auto* host = renderModule->GetRenderPassHost())
            host->DrawFallbackVulkan(*renderModule, delta);
    }
}

#endif
