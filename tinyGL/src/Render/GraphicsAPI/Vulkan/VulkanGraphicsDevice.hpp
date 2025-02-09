#pragma once
#include <vulkan/vulkan_core.h>

#include "../GraphicsDevice.hpp"

namespace Kong
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    // 目前需要graphics和present队列
    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    
    class VulkanGraphicsDevice : public GraphicsDevice
    {
    public:
        static VulkanGraphicsDevice* GetGraphicsDevice();
        
        VulkanGraphicsDevice();
        ~VulkanGraphicsDevice() override;
        
        GLFWwindow* Init(int width, int height) override;
        
        VkDevice GetDevice() const { return m_device; }
        VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue GetPresentQueue() const { return m_presentQueue; }
        VkCommandPool GetCommandPool() const {return m_commandPool;}
        VkSurfaceKHR GetSurface() const {return m_surface;}
        
        VkPhysicalDeviceProperties m_properties;

        SwapChainSupportDetails GetSwapChainSupport() {return QuerySwapChainSupport(m_physicalDevice);}
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices FindPhysicsQueueFamilies() {return FindQueueFamilies(m_physicalDevice);}
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        void CreateImageWithInfo(
            const VkImageCreateInfo &imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);
        
    private:
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkSurfaceKHR m_surface;
        VkPhysicalDevice m_physicalDevice {VK_NULL_HANDLE};
        VkDevice m_device;

        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;

        VkCommandPool m_commandPool;
        
        void InitVulkanDevice(GLFWwindow* window);

        // todo: 这里很多device相关的内容，后续考虑是否要移出去
        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface(GLFWwindow* window);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();

        // 检查是否支持validation layer
        bool CheckValidationLayerSupport();
        // 获取必要的扩展
        std::vector<const char*> GetRequiredExtensions();
        // 检查扩展是否安需求引入
        void CheckRequiredInstanceExtensions();
        // debug信息设定
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        
        // 检查设备的能力是否支持引擎的需求
        bool CheckDeivceSuitable(VkPhysicalDevice device);
        // 找到设备支持的队列（queue），包括graphics/compute/transfer/present
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        // 检查设备对扩展的支持
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        // 检查设备对swapchain的支持
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
        
        const std::vector<const char *> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        
#ifdef NDEBUG
        const bool m_enableValidationLayers {false};
#else
        const bool m_enableValidationLayers {true};
#endif
        
    };
    
}
