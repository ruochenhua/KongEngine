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

    struct BasicMaterialUbo
    {
        glm::vec4 albedo{0.2, 0.3, 0.1, 1.0};
        float specular_factor{1.0};
        float metallic {0.5};
        float roughness {0.5};
        float ambient {1.0};
    };
    
    class VulkanMaterialInfo : public RenderMaterialInfo
    {
    public:
        // descriptor set
        VulkanMaterialInfo();
        ~VulkanMaterialInfo() override;

        void AddMaterialByType(ETextureType textureType, std::weak_ptr<KongTexture> texture) override;
        void Initialize() override;
        
        std::vector<std::map<VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType, VkDescriptorSet>> m_descriptorSets;
        std::vector<std::unique_ptr<VulkanBuffer>> m_uboBuffers;  

        void CreateDescriptorSet(VulkanDescriptorSetLayout* descriptorSetLayout, VulkanDescriptorPool* descriptorPool);
    private:
        void CreateDescriptorBuffer();

        std::map<ETextureType, VkDescriptorImageInfo> m_imageInfoCache;
    };
}


#endif