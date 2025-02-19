#include "VulkanSwapChain.hpp"

#include <array>
#include <iostream>
#ifdef RENDER_IN_VULKAN
using namespace Kong;

VulkanSwapChain::VulkanSwapChain(VulkanGraphicsDevice& deviceRef, VkExtent2D extent)
    :m_deviceRef(deviceRef), m_windowExtent(extent)
{
    Init();
}

VulkanSwapChain::VulkanSwapChain(VulkanGraphicsDevice& deviceRef, VkExtent2D extent, std::shared_ptr<VulkanSwapChain> oldSwapChain)
    : m_deviceRef(deviceRef), m_windowExtent(extent), m_oldSwapChain(oldSwapChain)
{
    Init();
    oldSwapChain = nullptr;
}

VulkanSwapChain::~VulkanSwapChain()
{
    // 销毁资源
    VkDevice device = m_deviceRef.GetDevice();
    for (auto imageView :m_swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
    m_swapChainImageViews.clear();

    if (m_swapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }

    for (size_t i = 0; i < m_depthImages.size(); i++)
    {
        vkDestroyImageView(device, m_depthImageViews[i], nullptr);
        vkDestroyImage(device, m_depthImages[i], nullptr);
        vkFreeMemory(device, m_depthImageMemorys[i], nullptr);
    }

    for (auto framebuffer : m_swapChainFrameBuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkDestroyRenderPass(device, m_renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, m_imageAvailableSemaphores[i], nullptr);

        vkDestroyFence(device, m_inFlightFences[i], nullptr);
    }
}

VkFormat VulkanSwapChain::FindDepthFormat() const
{
    return m_deviceRef.FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkResult VulkanSwapChain::AcquireNextImage(uint32_t* imageIndex)
{
    // 等待CPU/GPU同步
    vkWaitForFences(
        m_deviceRef.GetDevice(),
        1,
        &m_inFlightFences[m_currentFrame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    // 获取下一个可用图像
    VkResult result = vkAcquireNextImageKHR(
        m_deviceRef.GetDevice(),
        m_swapChain,
        std::numeric_limits<uint64_t>::max(),
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        imageIndex);

    return result;
}

VkResult VulkanSwapChain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
{
    // 等待gpu同步
    if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_deviceRef.GetDevice(),1, &m_imagesInFlight[*imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }
    m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    /*
     * submitInfo.commandBufferCount 和 submitInfo.pCommandBuffers 指定了要提交的命令缓冲区数量和指针。
     */
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;
    /*
     * waitSemaphores 和 waitStages 表示在开始执行命令缓冲区之前，需要等待的信号量和等待的管线阶段。
     * 这里等待 m_imageAvailableSemaphores[m_currentFrame] 信号量，确保在有可用图像时才开始渲染
     */
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.pWaitSemaphores = waitSemaphores;

    /*
     * signalSemaphores 表示当命令缓冲区执行完成后，要触发的信号量。
     * 这里触发 m_renderFinishedSemaphores[m_currentFrame] 信号量，用于通知后续的呈现操作。
     */
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    /*
     * vkResetFences 函数将指定的围栏重置为未触发状态，以便在新的一帧中使用。
     * vkQueueSubmit 函数将配置好的 submitInfo 提交到图形队列中执行，并关联一个围栏，当命令缓冲区执行完成后，围栏会被触发。
     */
    vkResetFences(m_deviceRef.GetDevice(), 1, &m_inFlightFences[m_currentFrame]);
    if (vkQueueSubmit(m_deviceRef.GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer submission");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    /*
     * presentInfo.waitSemaphoreCount 和 presentInfo.pWaitSemaphores 表示在进行呈现操作之前，需要等待的信号量。
     * 这里等待 m_renderFinishedSemaphores[m_currentFrame] 信号量，确保渲染操作已经完成。
     */
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = imageIndex;

    /*
     * swapchains 和 presentInfo.pImageIndices 指定了要呈现的交换链和图像索引。
     * vkQueuePresentKHR 函数将指定的图像呈现到屏幕上。
     */
    auto result = vkQueuePresentKHR(m_deviceRef.GetPresentQueue(), &presentInfo);

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void VulkanSwapChain::Init()
{
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDepthResources();
    CreateFrameBuffers();
    CreateSyncObjects();
}

void VulkanSwapChain::CreateSwapChain()
{
    // 获取当前device对swapchain的支持信息
    SwapChainSupportDetails swapChainSupport = m_deviceRef.GetSwapChainSupport();

    // 选择合适的表面格式,定义swapchain的颜色空间和像素格式
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    // 选择合适的程序模式,决定图像如何呈现到屏幕上,如双缓冲,三缓冲等等
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    // swapchain的宽高
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    // 确定swapchain图像个数,imageCount数量不能超出
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;  // todo : check 3 image error
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_deviceRef.GetSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;          // 格式
    createInfo.imageColorSpace = surfaceFormat.colorSpace;  // 颜色空间
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // 用作颜色附件

    QueueFamilyIndices indices = m_deviceRef.FindPhysicsQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    /*
     * 获取图形队列族和呈现队列族的索引。graphics queue和present queue
     * 如果这两个队列族不同，则需要使用 VK_SHARING_MODE_CONCURRENT 共享模式，允许图像在不同队列族之间共享，并指定队列族索引。
     * 如果这两个队列族相同，则使用 VK_SHARING_MODE_EXCLUSIVE 独占模式，图像只能由一个队列族访问。
     */
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;   // 设定图像预变换(旋转90度, 镜像等等)
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;      // 合成透明度模式,这里设置为不透明

    createInfo.presentMode = presentMode;       // 设定呈现模式(之前获取过)
    createInfo.clipped = VK_TRUE;               // 是否允许裁切

    createInfo.oldSwapchain = m_oldSwapChain==nullptr ? VK_NULL_HANDLE : m_oldSwapChain->m_swapChain;   // 就swapchain资源,用于重新创建时的过度

    // 创建swapchain
    if (vkCreateSwapchainKHR(m_deviceRef.GetDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain");
    }

    // 创建swapchain图像
    vkGetSwapchainImagesKHR(m_deviceRef.GetDevice(), m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_deviceRef.GetDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

    // 保存swapchain的格式和尺寸
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

/*
 * 为交换链中的每个图像创建对应的图像视图（Image View）。
 * 在 Vulkan 中，图像视图定义了如何访问图像数据，它描述了图像的特定部分以及如何解释这些数据，是渲染过程中必不可少的环节。
 */
void VulkanSwapChain::CreateImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapChainImages[i];      // 图像句柄
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // 图像视图类型,这里是2D图像
        viewInfo.format = m_swapChainImageFormat;   // 图像格式,用保存的swapchainImage格式
        // 图像视图所涵盖的子资源范围
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;       // 图像的方面掩码,这里表示该图像视图用于访问颜色数据   
        viewInfo.subresourceRange.baseMipLevel = 0;             // 基础的mip级别
        viewInfo.subresourceRange.levelCount = 1;               // mip级别数量
        viewInfo.subresourceRange.baseArrayLayer = 0;           // 指定基础的数组层,设置为0表示从第一个数组层开始
        viewInfo.subresourceRange.layerCount = 1;               // 数组层数量

        /*
         * 在 Vulkan 中，图像（VkImage）可以是一个包含多个层的数组。
         * 每个数组层可以看作是一个独立的图像平面，它们具有相同的尺寸、格式和其他属性，但存储着不同的数据内容。
         * 数组层允许在一个图像对象中高效地组织和管理多个相关的图像，这种结构有助于提高内存使用效率和渲染性能。
         * 如立方体贴图可以有6个数组层;纹理数组可以有多个数组层
         */

        if (vkCreateImageView(m_deviceRef.GetDevice(), &viewInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views");
        }
    }
}

void VulkanSwapChain::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();
    m_swapChainDepthFormat = depthFormat;
    VkExtent2D swapchainExtent = GetSwapChainExtent();

    size_t imageCount = m_swapChainImages.size();
    m_depthImages.resize(imageCount);
    m_depthImageMemorys.resize(imageCount);
    m_depthImageViews.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        // 创建深度图像
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapchainExtent.width;
        imageInfo.extent.height = swapchainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        // 根据配置好的 imageInfo 创建深度图像，并分配设备本地内存（VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT），
        // 将创建的深度图像句柄存储在 m_depthImages[i] 中，分配的内存句柄存储在 m_depthImageMemorys[i] 中。
        m_deviceRef.CreateImageWithInfo(
            imageInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_depthImages[i],
            m_depthImageMemorys[i]);

        // 创建深度图像视图
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_depthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_deviceRef.GetDevice(), &viewInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views");
        }
    }
}

void VulkanSwapChain::CreateRenderPass()
{
    // 深度附件描述
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;        // 设定多重采样(1表示不采样)
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;           // 渲染通道开始时对深度附件数据的加载操作(这里表示开始时清空附件)
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;     // 渲染通道结束时对深度附件数据的加载操作(这里表示不关心)
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;    // 渲染通道开始和结束时对模板缓冲区的操作
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;          // 指定附件在渲染通道开始时的图像布局(这里表示不关心)
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // 指定附件在渲染通道结束时的图像布局(这里表示用于深度模板附近的最佳布局)

    // 深度附件引用:定义子通道中如何引用深度附件
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;      // 指定附件的索引, 表示是第二个附件(从0开始)
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;       // 指定子通道中使用附件时的图像布局(这里表示用于深度附件的最佳布局)

    // 颜色附件描述
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = GetSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     // 渲染通道结束时保存颜色数据,以便后续呈现
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // 表示该附件在渲染结束后用于呈现

    // 颜色附件引用
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;  // 表示第一个附件
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   // 表示用于颜色附件的最佳布局

    /*
     * 一个渲染通道可以包含多个子通道，每个子通道定义了一组特定的渲染操作，比如绘制几何体、进行后处理等。
     * 子通道允许在不切换渲染目标的情况下，对同一组附件执行多个渲染操作，从而减少不必要的内存操作和同步开销。
     */
    // 子通道描述:定义一个子通道，子通道是渲染通道中的一个渲染步骤。
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;    // 指定绑定的管线类型(这里表示绑定图形管线)
    subpass.colorAttachmentCount = 1;       // 颜色附件数量
    subpass.pColorAttachments = &colorAttachmentRef;    // 颜色附件引用的指针
    subpass.pDepthStencilAttachment = &depthAttachmentRef;  // 深度模板附件引用的指针

    /*
     * 子通道依赖用于描述子通道之间的执行顺序和数据依赖关系。
     * 它确保一个子通道在另一个子通道完成特定操作后才开始执行，从而保证渲染结果的正确性。
     */
    // 子通道依赖:定义子通道之间的依赖关系，确保渲染操作按正确顺序执行。
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;    // 指定源子通道(这里表示渲染通道外部的操作)
    dependency.srcAccessMask = 0;   // 指定源子通道的访问掩码(这里表示不关心)
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 指定管线阶段(这里表示包括颜色附件输出阶段和早期片段测试阶段)
    
    dependency.dstSubpass = 0;  // 指定目标子通道,这里表示当前子通道
    dependency.dstStageMask= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 指定目标子通道的管线阶段,和源子通道相同
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // 指定目标子通道访问掩码,包括颜色附件写入和深度模板附件写入操作

    // 创建渲染通道
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_deviceRef.GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass");
    }
}

void VulkanSwapChain::CreateFrameBuffers()
{
    m_swapChainFrameBuffers.resize(m_swapChainImages.size());
    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {m_swapChainImageViews[i], m_depthImageViews[i]};

        VkExtent2D swapChainExtent = GetSwapChainExtent();
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
            m_deviceRef.GetDevice(),
            &framebufferInfo,
            nullptr,
            &m_swapChainFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void VulkanSwapChain::CreateSyncObjects()
{
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_deviceRef.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i])
            != VK_SUCCESS ||
            vkCreateSemaphore(m_deviceRef.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i])
            != VK_SUCCESS ||
            vkCreateFence(m_deviceRef.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fence");
        }
    }
}

VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    /* mailbox同步方法和fifo(v-sync)方法
   * 区别在于在fifo模式下，gpu完成渲染后到屏幕刷新的时间之间，gpu处于idle状态
   * mailbox模式下，gpu从不idle，会一直渲染覆盖旧的image，屏幕刷新的时候取状态最新的image
   * mailbox模式比fifo模式有更小的延迟，对用户输入反馈更加及时，但也有更高的性能消耗
   * immediate方法则是不等待显示器刷新，会出现撕裂
  */
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            std::cout << "Present mode: Mailbox\n";
            return availablePresentMode;
        }
    }

    // for (const auto &availablePresentMode : availablePresentModes) {
    //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     std::cout << "Present mode: Immediate" << std::endl;
    //     return availablePresentMode;
    //   }
    // }

    std::cout << "Present mode: V-Sync\n";
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = m_windowExtent;
        actualExtent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
#endif