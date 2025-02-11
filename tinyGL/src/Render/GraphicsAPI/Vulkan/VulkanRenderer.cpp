#include "VulkanRenderer.hpp"

#include <array>

#include "Window.hpp"

using namespace Kong;

VulkanRenderer::VulkanRenderer(VulkanGraphicsDevice& device)
    : m_deviceRef(device)
{
    CreateCommandBuffers();
    RecreateSwapChain();
}

VulkanRenderer::~VulkanRenderer()
{
    FreeCommandBuffers();
}

int VulkanRenderer::GetFrameIndex() const
{
    assert(m_isFrameStarted && "Can not get frame index when frame not in progress");
    return m_currentFrameIndex;
}

void VulkanRenderer::BeginFrame()
{
    assert(!m_isFrameStarted && "cannot begin frame when frame already in progress");

    auto result = m_swapChain->AcquireNextImage(&m_currentImageIndex);
    // 可能是窗口有变化，需要重新创建swapchain
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    m_isFrameStarted = true;

    // commandbuffer开始
    auto commandBuffer = GetCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer");
    }
}

void VulkanRenderer::EndFrame()
{
    assert(m_isFrameStarted && "cannot end frame when frame not in progress");
    auto commandBuffer = GetCurrentCommandBuffer();

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to end command buffer");
    }

    auto result = m_swapChain->SubmitCommandBuffers(&commandBuffer, &m_currentImageIndex);
    // 如果窗口有变化，需要重新创建swapchain
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer frame");
    }

    m_isFrameStarted = false;
    m_currentFrameIndex = (m_currentFrameIndex + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
    
}

void VulkanRenderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(m_isFrameStarted && "cannot beginSwapChainRenderPass when frame not in progress");
    assert(commandBuffer == GetCurrentCommandBuffer() && "cannot begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapChain->GetRenderPass();
    renderPassInfo.framebuffer = m_swapChain->GetFrameBuffer(m_currentFrameIndex);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    // 对应framebuffer和render pass的设定，attachment0是color，attachment1是depth，所以只需要设置对应的颜色和depthStencil的clear值
    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    // inline类型代表直接执行command buffer中的渲染指令，不存在引用其他command buffer
    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS代表有引用的情况，两种不能混合使用
    // 启用render pass
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(m_swapChain->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0,0}, m_swapChain->GetSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanRenderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(m_isFrameStarted && "cannot endSwapChainRenderPass when frame not in progress");
    assert(commandBuffer == GetCurrentCommandBuffer() && "cannot end render pass on command buffer from a different frame");
    
    vkCmdEndRenderPass(commandBuffer);
}

VkCommandBuffer VulkanRenderer::GetCurrentCommandBuffer() const
{
    assert(m_isFrameStarted && "cannot get command buffer when frame not in progress");
    return m_commandBuffers[m_currentFrameIndex];
}

void VulkanRenderer::CreateCommandBuffers()
{
    m_commandBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    /* level有两种: primary和secondary
    * primary可以送到Device Graphics Queue执行，但是不能被其他Command Buffer引用
    * secondary不能送到Device Graphics Queue执行，但是可以被其他command buffer引用
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_deviceRef.GetCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_deviceRef.GetDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanRenderer::FreeCommandBuffers()
{
    vkFreeCommandBuffers(m_deviceRef.GetDevice(), m_deviceRef.GetCommandPool(),
        static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

    m_commandBuffers.clear();
}

void VulkanRenderer::RecreateSwapChain()
{
    auto window_size = KongWindow::GetWindowModule().windowSize;
    
    VkExtent2D window_extent = {0, 0};

    // 可能在全屏或者最小化之类的，窗口大小为0，要等待事件结束
    while (window_size.x == 0 || window_size.y == 0)
    {
        window_size = KongWindow::GetWindowModule().windowSize;
        glfwWaitEvents();
    }

    window_extent = {static_cast<uint32_t>(window_size.x), static_cast<uint32_t>(window_size.y)};

    if (m_swapChain == nullptr)
    {
        m_swapChain = std::make_unique<VulkanSwapChain>(m_deviceRef, window_extent);
    }
    else
    {
        std::shared_ptr<VulkanSwapChain> oldSwapChain = std::move(m_swapChain);
        m_swapChain = std::make_unique<VulkanSwapChain>(m_deviceRef, window_extent, oldSwapChain);

        if (!oldSwapChain->CompareSwapChainFormats(*m_swapChain.get()))
        {
            throw std::runtime_error("failed to create swap chain!");
        }
    }
}
