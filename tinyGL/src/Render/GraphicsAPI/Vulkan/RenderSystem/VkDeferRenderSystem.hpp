#pragma once
#include "Render/Abstraction/RHIRenderSubsystems.hpp"
#include "VkModelRenderSystem.hpp"

#ifdef RENDER_IN_VULKAN

namespace Kong
{
    class CQuadShape;
}

namespace Kong
{
    class VkDeferRenderSystem : public VkModelRenderSystem, public IRHIRenderSubsystem
    {
    public:
        RHISubsystemKind GetRHISubsystemKind() const noexcept override { return RHISubsystemKind::Deferred; }

        VkDeferRenderSystem();
        ~VkDeferRenderSystem() override;
        
        VkDeferRenderSystem(const VkDeferRenderSystem&) = delete;
        VkDeferRenderSystem& operator=(const VkDeferRenderSystem&) = delete;

        void Draw(const FrameInfo& frameInfo) override;
    
    private:
        void CreatePipeline();
        void CreateRenderPass();
        void CreateDescriptorSets();
        
        // ��ʵӦ�ÿ��Ժ�simple render system����һ��framebuffer
        void CreateFrameBuffers();
        void CreateDeferColorDescriptorSetLayout();
        void CreateDeferColorPipelineLayout();
        void CreateDeferGeometryTexture();
        
        //descriptor set layout 0=ȫ�����ݣ�1=�����������ݣ�2=��ͼ

        // defer�ڶ��׶εĸ������ã������׶���һ��renderpass������subpass
        VkPipelineLayout m_deferColorPipelineLayout {VK_NULL_HANDLE};
        std::unique_ptr<VulkanPipeline> m_deferColorPipeline;
        std::vector<VkDescriptorSet> m_deferColorDescriptorSets;
       
        std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> m_deferColorDescriptorSetLayouts;

        std::unique_ptr<VulkanTexture> m_positionTexture;
        std::unique_ptr<VulkanTexture> m_normalTexture;
        std::unique_ptr<VulkanTexture> m_albedoTexture;
        std::unique_ptr<VulkanTexture> m_ormTexture;

        std::unique_ptr<CQuadShape> m_quadShape;
    };
}


#endif