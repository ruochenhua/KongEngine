#include "VkModelRenderSystem.hpp"

#include <array>

#include "Actor.hpp"
#include "Scene.hpp"
#include "Render/RenderModule.hpp"
#include "Render/Resource/Texture.hpp"

using namespace Kong;

#ifdef RENDER_IN_VULKAN

VkModelRenderSystem::VkModelRenderSystem()
{
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreateTextures();
}

VkModelRenderSystem::~VkModelRenderSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    if (m_pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }
}

void VkModelRenderSystem::Draw(const FrameInfo& frameInfo)
{
}


void VkModelRenderSystem::UpdateMeshUBO(const FrameInfo& frameInfo)
{
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto mesh_component = actor->GetComponent<CMeshComponent>();
        if (!mesh_component)
        {
            continue;
        }

        auto mesh_shader = mesh_component->shader_data;
        if (dynamic_pointer_cast<DeferInfoShader>(mesh_shader) || dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader))
        {
            continue;
        }

        mesh_component->UpdateMeshUBO(frameInfo);
    }
}

void VkModelRenderSystem::CreateMeshDescriptorSet()
{    
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto mesh_component = actor->GetComponent<CMeshComponent>();
        if (!mesh_component)
        {
            continue;
        }

        auto mesh_shader = mesh_component->shader_data;
        if (dynamic_pointer_cast<DeferInfoShader>(mesh_shader) || dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader))
        {
            continue;
        }

        mesh_component->CreateMeshDescriptorSet(m_descriptorSetLayout,
            KongRenderModule::GetRenderModule().m_descriptorPool.get());
    }
}

VulkanTexture* VkModelRenderSystem::GetColorTexture() const
{
    return m_sceneTexture.get();
}

VulkanTexture* VkModelRenderSystem::GetDepthTexture() const
{
    return m_depthTexture.get();
}

void VkModelRenderSystem::CreateDescriptorSetLayout()
{
    // �����Ĳ�����Ϣ��ֻ��Ҫ����fragment shader
    auto basicMaterialLayout = VulkanDescriptorSetLayout::Builder()
    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
    .Build();
    basicMaterialLayout->m_usage = VulkanDescriptorSetLayout::BasicMaterial;
    m_descriptorSetLayout.push_back(std::move(basicMaterialLayout));
    
    auto textureLayout = VulkanDescriptorSetLayout::Builder()
    // image ��ͼ����
    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // albedo
    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // normal
    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // roughness
    .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // metallic
    .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // ambient_occlusion
    .Build();
    textureLayout->m_usage = VulkanDescriptorSetLayout::Texture;
    m_descriptorSetLayout.push_back(std::move(textureLayout));
}

void VkModelRenderSystem::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    // set放在vector中，从set0,set1,set2 ...
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    // descriptor set layout
    descriptorSetLayouts.push_back(KongRenderModule::GetRenderModule().m_descriptorLayout->GetDescriptorSetLayout());
    for (auto& layout : m_descriptorSetLayout)
    {
        descriptorSetLayouts.push_back(layout->GetDescriptorSetLayout());
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void VkModelRenderSystem::CreateTextures()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    auto extent = m_swapChain->GetSwapChainExtent();

    // color image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_swapChain->GetSwapChainImageFormat();
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    m_sceneTexture = make_unique<VulkanTexture>(imageInfo);
    m_sceneTexture->CreateImageView(m_swapChain->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
    m_sceneTexture->CreateTextureSampler();
    
    // depth image
    VkFormat depthFormat = m_swapChain->FindDepthFormat();
    {
        // �������ͼ��
        VkImageCreateInfo depthImageInfo = {};
        depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageInfo.extent.width = extent.width;
        depthImageInfo.extent.height = extent.height;
        depthImageInfo.extent.depth = 1;
        depthImageInfo.mipLevels = 1;
        depthImageInfo.arrayLayers = 1;
        depthImageInfo.format = depthFormat;
        depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        depthImageInfo.flags = 0;

        // �������úõ� imageInfo �������ͼ�񣬲������豸�����ڴ棨VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT����
        // �����������ͼ�����洢�� m_depthImages[i] �У�������ڴ����洢�� m_depthImageMemorys[i] �С�
        m_depthTexture = make_unique<VulkanTexture>(depthImageInfo);
        m_depthTexture->CreateImageView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
}


#endif
