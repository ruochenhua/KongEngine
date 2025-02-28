#pragma once
#include "VulkanDescriptor.hpp"
#ifdef RENDER_IN_VULKAN
#include "Render/RenderCommon.hpp"

namespace Kong
{
    class VulkanBuffer;

    class VulkanRenderInfo : public RenderInfo
    {
    public:
        VulkanRenderInfo();
        
        void Draw(void* commandBuffer) override;
        void InitRenderInfo() override;
    };

    class VulkanMaterialInfo : public RenderMaterialInfo
    {
    public:
        // descriptor set
        VulkanMaterialInfo();
        ~VulkanMaterialInfo() override;

        void AddMaterialByType(ETextureType textureType, std::weak_ptr<KongTexture> texture) override;
        void Initialize() override;
        
        std::vector<std::map<VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType, VkDescriptorSet>> m_discriptorSets;
        std::vector<std::unique_ptr<VulkanBuffer>> m_uboBuffers;    // todo: ubo应该是全场景公用的吧

        void CreateDescriptorSet(VulkanDescriptorSetLayout* descriptorSetLayout, VulkanDescriptorPool* descriptorPool);
    private:
        void CreateDescriptorBuffer();

        std::map<ETextureType, VkDescriptorImageInfo> m_imageInfoCache;
    };
}


#endif