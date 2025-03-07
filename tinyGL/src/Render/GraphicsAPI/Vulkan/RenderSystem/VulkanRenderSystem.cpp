#include "VulkanRenderSystem.hpp"

#include <array>

#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"

using namespace Kong;
#ifdef RENDER_IN_VULKAN

VulkanRenderSystem::VulkanRenderSystem(VulkanSwapChain* swapChain)
    :m_swapChain(swapChain)
{
}

void VulkanRenderSystem::BeginRenderPass(VkCommandBuffer commandBuffer)
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;

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

void VulkanRenderSystem::EndRenderPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

#endif