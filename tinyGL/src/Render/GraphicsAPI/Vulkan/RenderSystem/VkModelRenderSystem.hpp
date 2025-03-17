#pragma once
#include "VulkanRenderSystem.hpp"
#ifdef RENDER_IN_VULKAN
namespace Kong
{
    class VulkanTexture;
    class KongRenderModule;
    
    class VkModelRenderSystem : public VulkanRenderSystem
    {
    public:
        struct SimplePushConstantData
        {
            glm::mat4 modelMatrix{1.0f};
        };
        
        VkModelRenderSystem();
        ~VkModelRenderSystem() override;

        VkModelRenderSystem(const VkModelRenderSystem&) = delete;
        VkModelRenderSystem& operator=(const VkModelRenderSystem&) = delete;

        virtual void Draw(const FrameInfo& frameInfo);
        void UpdateMeshUBO(const FrameInfo& frameInfo);
        void CreateMeshDescriptorSet();

        VkImage GetColorImage() const;
        VkImageView GetColorImageView() const;
        VkSampler GetSampler() const;
        VkFramebuffer GetFrameBuffer() const { return m_framebuffer; }
    protected:
        // *simple model render��defer renderģ�Ͳ��descriptor set layoutӦ����ͨ�õ�
        void CreateDescriptorSetLayout();
        void CreatePipelineLayout();
        void CreateTextures();

        std::unique_ptr<VulkanTexture> m_sceneTexture;
        std::unique_ptr<VulkanTexture> m_depthTexture;        
    };
    
}

#endif