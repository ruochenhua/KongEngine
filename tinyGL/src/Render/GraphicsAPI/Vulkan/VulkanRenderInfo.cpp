#include "VulkanRenderInfo.hpp"

#include "VulkanDescriptor.hpp"
#include "VulkanSwapChain.hpp"
#include "Render/RenderModule.hpp"

#ifdef RENDER_IN_VULKAN

#include "VulkanBuffer.hpp"
using namespace Kong;

VulkanRenderInfo::VulkanRenderInfo()
{
    material = std::make_shared<VulkanMaterialInfo>();
    
}

void VulkanRenderInfo::Draw(void* commandBuffer)
{
    auto cb = static_cast<VkCommandBuffer>(commandBuffer);

    if (!vertex_buffer->IsValid())
    {
        return;
    }
    
    vertex_buffer->Bind(commandBuffer);
    if (index_buffer && index_buffer->IsValid())
    {
        index_buffer->Bind(commandBuffer);
        
        vkCmdDrawIndexed(cb, static_cast<uint32_t>(m_Index.size()), 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(cb, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }
}

void VulkanRenderInfo::InitRenderInfo()
{
    vertex_buffer = std::make_unique<VulkanBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), vertices.size(), &vertices[0]);
    
    if(!m_Index.empty())
    {
        index_buffer = make_unique<VulkanBuffer>();
        index_buffer->Initialize(INDEX_BUFFER, sizeof(unsigned int), m_Index.size(), &m_Index[0]);
    }
}

VulkanMaterialInfo::VulkanMaterialInfo()
{
    CreateDescriptorBuffer();
    
    auto nullTex = dynamic_cast<VulkanTexture*>(KongRenderModule::GetNullTex());
    m_imageInfoCache.emplace(ETextureType::diffuse, VkDescriptorImageInfo{nullTex->m_sampler, nullTex->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    m_imageInfoCache.emplace(ETextureType::normal, VkDescriptorImageInfo{nullTex->m_sampler, nullTex->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    m_imageInfoCache.emplace(ETextureType::roughness, VkDescriptorImageInfo{nullTex->m_sampler, nullTex->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    m_imageInfoCache.emplace(ETextureType::metallic, VkDescriptorImageInfo{nullTex->m_sampler, nullTex->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    m_imageInfoCache.emplace(ETextureType::ambient_occlusion, VkDescriptorImageInfo{nullTex->m_sampler, nullTex->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
}

VulkanMaterialInfo::~VulkanMaterialInfo()
{
    
}

void VulkanMaterialInfo::AddMaterialByType(ETextureType textureType, std::weak_ptr<KongTexture> texture)
{
    RenderMaterialInfo::AddMaterialByType(textureType, texture);
    auto vulkanTexture = dynamic_cast<VulkanTexture*>(texture.lock().get());
    // 还需要创建对应的ImageInfo，在create descriptor set可以直接用到
    if (vulkanTexture->m_imageView != VK_NULL_HANDLE && m_imageInfoCache.find(textureType) != m_imageInfoCache.end())
    {
        m_imageInfoCache[textureType].imageView = vulkanTexture->m_imageView;
        m_imageInfoCache[textureType].sampler = vulkanTexture->m_sampler;
    }
}

void VulkanMaterialInfo::Initialize()
{
    RenderMaterialInfo::Initialize();
}

void VulkanMaterialInfo::CreateDescriptorBuffer()
{
    m_uboBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < m_uboBuffers.size(); ++i)
    {
        m_uboBuffers[i] = std::make_unique<VulkanBuffer>();
        m_uboBuffers[i]->Initialize(UNIFORM_BUFFER, sizeof(BasicMaterialUbo), 1);
        m_uboBuffers[i]->Map();
    }
}

void VulkanMaterialInfo::CreateDescriptorSet(VulkanDescriptorSetLayout* descriptorSetLayout, VulkanDescriptorPool* descriptorPool)
{
    m_descriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < m_descriptorSets.size(); i++)
    {
        switch (descriptorSetLayout->m_usage)
        {
        case VulkanDescriptorSetLayout::BasicMaterial:
            {
                VkDescriptorSet newSet;
                auto bufferInfo = m_uboBuffers[i]->DescriptorInfo();
                VulkanDescriptorWriter(*descriptorSetLayout, *descriptorPool)
                .WriteBuffer(0, &bufferInfo)
                .Build(newSet);
                m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::BasicMaterial, newSet);
            }
            break;
        case VulkanDescriptorSetLayout::Texture:
            {
                VkDescriptorSet newSet;
                VulkanDescriptorWriter(*descriptorSetLayout, *descriptorPool)
                .WriteImage(0, &m_imageInfoCache[ETextureType::diffuse])
                .WriteImage(1, &m_imageInfoCache[ETextureType::normal])
                .WriteImage(2, &m_imageInfoCache[ETextureType::roughness])
                .WriteImage(3, &m_imageInfoCache[ETextureType::metallic])
                .WriteImage(4, &m_imageInfoCache[ETextureType::ambient_occlusion])
                .Build(newSet);
                m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::Texture, newSet);
            }
            break;
        default:
            break;
        }
    }
}

#endif
