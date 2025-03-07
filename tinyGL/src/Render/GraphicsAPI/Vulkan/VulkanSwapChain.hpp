#pragma once
#include "VulkanGraphicsDevice.hpp"
#ifdef RENDER_IN_VULKAN
namespace Kong
{
    class VulkanSwapChain
    {
    public:
        // 先默认使用两个frame
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VulkanSwapChain(VulkanGraphicsDevice &deviceRef, VkExtent2D extent);
        // 从老swapchain创建新swapchain，在重设窗口时使用
        VulkanSwapChain(VulkanGraphicsDevice &deviceRef, VkExtent2D extent, std::shared_ptr<VulkanSwapChain> oldSwapChain);
        ~VulkanSwapChain();

        // 不可复制和赋值
        VulkanSwapChain(const VulkanSwapChain &) = delete;
        VulkanSwapChain &operator=(const VulkanSwapChain &) = delete;

        VkFramebuffer GetFrameBuffer(int index) const { return m_swapChainFrameBuffers[index]; }
        VkRenderPass GetRenderPass() const { return m_renderPass; }
        VkImageView GetImageView(int index) const { return m_swapChainImageViews[index]; }
        size_t GetImageCount() const { return m_swapChainImages.size(); }
        VkFormat GetSwapChainImageFormat() const { return m_swapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() const { return m_swapChainExtent; }
        float GetExtentAspectRatio() const
        {
            return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
        }

        VkFormat FindDepthFormat() const;
        bool CompareSwapChainFormats(const VulkanSwapChain &other) const
        {
            return m_swapChainImageFormat == other.m_swapChainImageFormat &&
                m_swapChainDepthFormat == other.m_swapChainDepthFormat;
        }
        
        VkResult AcquireNextImage(uint32_t *imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
        
    private:
        void Init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFrameBuffers();
        void CreateSyncObjects();

        
        // 帮助函数
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        
        // image格式
        VkFormat m_swapChainImageFormat;
        VkFormat m_swapChainDepthFormat;
        
        // swapchain尺寸(不一定和windows尺寸一样)
        VkExtent2D m_swapChainExtent;

        std::vector<VkFramebuffer> m_swapChainFrameBuffers;
        VkRenderPass m_renderPass;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        
        std::vector<VkImage> m_depthImages;
        std::vector<VkDeviceMemory> m_depthImageMemorys;
        std::vector<VkImageView> m_depthImageViews;

        
        VulkanGraphicsDevice& m_deviceRef;
        VkExtent2D m_windowExtent;

        // 当前swapchain
        VkSwapchainKHR m_swapChain;
        // 旧swapchain
        std::shared_ptr<VulkanSwapChain> m_oldSwapChain;

        // 信号量同步
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        // 栅栏同步
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkFence> m_imagesInFlight;
        
        size_t m_currentFrame {0};
    };
    
}
#endif