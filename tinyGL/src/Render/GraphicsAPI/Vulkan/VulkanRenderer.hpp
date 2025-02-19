#pragma once
#include "VulkanGraphicsDevice.hpp"
#include "VulkanSwapChain.hpp"
#include "Render/GraphicsAPI/Renderer.hpp"

namespace Kong
{
    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer();
        ~VulkanRenderer() override;

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        int GetFrameIndex() const;

        void BeginFrame() override;
        void EndFrame() override;

        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

        bool IsFrameInProgress() const {return m_isFrameStarted;}
        VkCommandBuffer GetCurrentCommandBuffer() const;

        VkRenderPass GetSwapChainRenderPass() const {return m_swapChain->GetRenderPass();}
        float GetAspectRatio() const {return m_swapChain->GetExtentAspectRatio();}
    private:
        void CreateCommandBuffers();
        void FreeCommandBuffers();

        void RecreateSwapChain();

        std::unique_ptr<VulkanSwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex {0};
        int m_currentFrameIndex {0};
        bool m_isFrameStarted {false};
    };
    
}
