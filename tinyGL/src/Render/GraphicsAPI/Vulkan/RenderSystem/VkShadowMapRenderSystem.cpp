#include "VkShadowMapRenderSystem.h"

#include "Actor.hpp"
#include "Scene.hpp"
#include "Component/LightComponent.h"

using namespace Kong;


VkShadowMapRenderSystem::VkShadowMapRenderSystem()
{
    m_directLightShadowMapRenderSystem = make_unique<VkDirectLightShadowMapRenderSystem>();
}

VkShadowMapRenderSystem::~VkShadowMapRenderSystem()
{
}

void VkShadowMapRenderSystem::Draw(const FrameInfo& frameInfo)
{
    m_directLightShadowMapRenderSystem->Draw(frameInfo);
}

VkDirectLightShadowMapRenderSystem::~VkDirectLightShadowMapRenderSystem()
{
}

void VkDirectLightShadowMapRenderSystem::Draw(const FrameInfo& frameInfo)
{
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto light_component = actor->GetComponent<CDirectionalLightComponent>();
        if (!light_component)
        {
            continue;
        }

        // TODO: 渲染光源阴影贴图
    }
}

void VkDirectLightShadowMapRenderSystem::CreateRenderPass()
{
    
}

void VkDirectLightShadowMapRenderSystem::CreateDescriptorSetLayout()
{
}

void VkDirectLightShadowMapRenderSystem::CreatePipeline()
{
}

void VkDirectLightShadowMapRenderSystem::CreatePipelineLayout()
{
}

void VkDirectLightShadowMapRenderSystem::CreateDescriptorSet()
{
}

void VkDirectLightShadowMapRenderSystem::CreateFramebuffer()
{
}
