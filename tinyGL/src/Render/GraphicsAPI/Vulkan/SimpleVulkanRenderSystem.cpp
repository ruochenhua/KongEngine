#include "SimpleVulkanRenderSystem.hpp"

#include <array>

#include "Actor.hpp"
#include "Scene.hpp"
#include "VulkanDescriptor.hpp"
#include "VulkanSwapChain.hpp"
#include "Render/RenderModule.hpp"

using namespace Kong;
#ifdef RENDER_IN_VULKAN

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.0f};
    int use_texture {0};
};

SimpleVulkanRenderSystem::SimpleVulkanRenderSystem(VulkanSwapChain* swapChain)
    :m_swapChain(swapChain)
{
    CreateRenderPass();
    CreateFrameBuffers();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
}

SimpleVulkanRenderSystem::~SimpleVulkanRenderSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(device, m_renderPass, nullptr);

    vkDestroyFramebuffer(device, m_framebuffer, nullptr);

    vkDestroyImage(device, m_image, nullptr);
    vkFreeMemory(device, m_imageMemory, nullptr);
    vkDestroyImageView(device, m_imageView, nullptr);

    vkDestroyImage(device, m_depthImage, nullptr);
    vkFreeMemory(device, m_depthImageMemory, nullptr);
    vkDestroyImageView(device, m_depthImageView, nullptr);

    vkDestroySampler(device, m_sampler, nullptr);
}

void SimpleVulkanRenderSystem::UpdateMeshUBO(const FrameInfo& frameInfo)
{
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto mesh_component = actor->GetComponent<CMeshComponent>();
        if (!mesh_component)
        {
            continue;
        }

        auto mesh_shader = mesh_component->shader_data;
        if (dynamic_pointer_cast<DeferInfoShader>(mesh_shader) || dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader))
        {
            continue;
        }

        mesh_component->UpdateMeshUBO(frameInfo);
    }
}

void SimpleVulkanRenderSystem::CreateMeshDescriptorSet()
{
    
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto mesh_component = actor->GetComponent<CMeshComponent>();
        if (!mesh_component)
        {
            continue;
        }

        auto mesh_shader = mesh_component->shader_data;
        if (dynamic_pointer_cast<DeferInfoShader>(mesh_shader) || dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader))
        {
            continue;
        }

        mesh_component->CreateMeshDescriptorSet(m_descriptorSetLayout,
            KongRenderModule::GetRenderModule().m_descriptorPool.get());
    }
}

void SimpleVulkanRenderSystem::Draw(const FrameInfo& frameInfo, VkCommandBuffer commandBuffer)
{
    BeginRenderPass(commandBuffer);
    
    m_pipeline->Bind(frameInfo.commandBuffer);
    int frameIndex = frameInfo.frameIndex;
    
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto mesh_component = actor->GetComponent<CMeshComponent>();
        if (!mesh_component)
        {
            continue;
        }

        auto mesh_shader = mesh_component->shader_data;
        if (dynamic_pointer_cast<DeferInfoShader>(mesh_shader) || dynamic_pointer_cast<DeferredTerrainInfoShader>(mesh_shader))
        {
            continue;
        }
        
        SimplePushConstantData push{};
        push.modelMatrix = actor->GetModelMatrix();
        push.use_texture = 0;

        vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(SimplePushConstantData), &push);
        
        mesh_component->Draw(frameInfo, m_pipelineLayout);
    }

    EndRenderPass(commandBuffer);
}

void SimpleVulkanRenderSystem::BeginRenderPass(VkCommandBuffer commandBuffer)
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    // 对应framebuffer和render pass的设定，attachment0是color，attachment1是depth，所以只需要设置对应的颜色和depthStencil的clear值
    clearValues[0].color = { 0.f, 0.f, 0.f, 1.0f };
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

void SimpleVulkanRenderSystem::EndRenderPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void SimpleVulkanRenderSystem::CreateDescriptorSetLayout()
{
    // 基础的材质信息，只需要给到fragment shader
    auto basicMaterialLayout = VulkanDescriptorSetLayout::Builder()
    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
    .Build();
    basicMaterialLayout->m_usage = VulkanDescriptorSetLayout::BasicMaterial;
    m_descriptorSetLayout.push_back(std::move(basicMaterialLayout));
    
    auto textureLayout = VulkanDescriptorSetLayout::Builder()
    // image 贴图数据
    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // albedo
    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // normal
    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // roughness
    .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // metallic
    .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // ambient_occlusion
    .Build();
    textureLayout->m_usage = VulkanDescriptorSetLayout::Texture;
    m_descriptorSetLayout.push_back(std::move(textureLayout));
}

void SimpleVulkanRenderSystem::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    // set按顺序存在vector中，set0,set1,set2 ...
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    // 先放全局的descriptor set layout
    descriptorSetLayouts.push_back(KongRenderModule::GetRenderModule().m_descriptorLayout->GetDescriptorSetLayout());
    for (auto& layout : m_descriptorSetLayout)
    {
        descriptorSetLayouts.push_back(layout->GetDescriptorSetLayout());
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    // 用于将一些小量的数据送到shader中
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void SimpleVulkanRenderSystem::CreatePipeline()
{
    assert(m_pipelineLayout != nullptr && "pipelineLayout is null");
    
    // 使用swapchain的大小而不是Windows的，因为这两个有可能不是一一对应
    PipelineConfigInfo pipelineConfig{};
    VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = m_renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {vs, "shader/Vulkan/simple_shader.vulkan.vert.spv"},
        {fs, "shader/Vulkan/simple_shader.vulkan.frag.spv"}},
        pipelineConfig);
}

void SimpleVulkanRenderSystem::CreateRenderPass()
{
     // 深度附件描述
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_swapChain->FindDepthFormat();
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
    colorAttachment.format = m_swapChain->GetSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     // 渲染通道结束时保存颜色数据,以便后续呈现
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // 表示该附件在渲染结束后用于呈现
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 表示该附件用于shader的读入（喂给后处理）

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

    if (vkCreateRenderPass(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass");
    }
}

void SimpleVulkanRenderSystem::CreateFrameBuffers()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    auto extent = m_swapChain->GetSwapChainExtent();

    // color image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_swapChain->GetSwapChainImageFormat();
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    VulkanGraphicsDevice::GetGraphicsDevice()->CreateImageWithInfo(imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_swapChain->GetSwapChainImageFormat();
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image views");
    }

    // depth image
    VkFormat depthFormat = m_swapChain->FindDepthFormat();

    {
        // 创建深度图像
        VkImageCreateInfo depthImageInfo = {};
        depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageInfo.extent.width = extent.width;
        depthImageInfo.extent.height = extent.height;
        depthImageInfo.extent.depth = 1;
        depthImageInfo.mipLevels = 1;
        depthImageInfo.arrayLayers = 1;
        depthImageInfo.format = depthFormat;
        depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        depthImageInfo.flags = 0;

        // 根据配置好的 imageInfo 创建深度图像，并分配设备本地内存（VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT），
        // 将创建的深度图像句柄存储在 m_depthImages[i] 中，分配的内存句柄存储在 m_depthImageMemorys[i] 中。
        VulkanGraphicsDevice::GetGraphicsDevice()->CreateImageWithInfo(
            depthImageInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_depthImage,
            m_depthImageMemory);

        // 创建深度图像视图
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_depthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &m_depthImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views");
        }
    }
    
    VkFramebufferCreateInfo framebufferInfo = {};
    std::array<VkImageView, 2> attachments = {m_imageView, m_depthImageView};

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
        device,
        &framebufferInfo,
        nullptr,
        &m_framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer");
    }

    // 创建sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 5.0f;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    if (vkCreateSampler(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

#endif
