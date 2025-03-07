#include "VkSkyBoxRenderSystem.hpp"

#include <array>

#include "Render/RenderModule.hpp"

using namespace Kong;
#ifdef RENDER_IN_VULKAN
VulkanSkyBoxRenderSystem::VulkanSkyBoxRenderSystem(const VulkanSkyBoxCreateInfo& createInfo)
    : VulkanRenderSystem(createInfo.swapChain)
{
    // !frame buffer应该和之前的渲染system输出到的framebuffer是一样的
    // 可能是simple Render，可能是defer Render
    m_framebuffer = createInfo.frameBuffer;
    m_boxShape = make_unique<CBoxShape>();

    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
    CreateCubeImage();
    CreateDescriptorSet(createInfo);
}

VulkanSkyBoxRenderSystem::~VulkanSkyBoxRenderSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

void VulkanSkyBoxRenderSystem::Draw(const FrameInfo& frameInfo)
{
    BeginRenderPass(frameInfo.commandBuffer);

    EndRenderPass(frameInfo.commandBuffer);
}

void VulkanSkyBoxRenderSystem::CreateDescriptorSetLayout()
{
    auto textureLayout = VulkanDescriptorSetLayout::Builder()
    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
    .Build();
    textureLayout->m_usage = VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::Texture;
    m_descriptorSetLayout.push_back(std::move(textureLayout));
}

void VulkanSkyBoxRenderSystem::CreatePipelineLayout()
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.push_back(KongRenderModule::GetRenderModule().m_descriptorLayout->GetDescriptorSetLayout());
    for (auto& layout : m_descriptorSetLayout)
    {
        descriptorSetLayouts.push_back(layout->GetDescriptorSetLayout());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void VulkanSkyBoxRenderSystem::CreatePipeline()
{
    assert(m_pipelineLayout != nullptr && "pipelineLayout is null");
    
    PipelineConfigInfo pipelineConfig{};
    VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;    // 这里关掉深度写入，深度测试保持开启
    
    pipelineConfig.renderPass = m_renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {vs, "shader/Vulkan/skybox.vulkan.vert.spv"},
        {fs, "shader/Vulkan/skybox.vulkan.frag.spv"}},
        pipelineConfig);
}

void VulkanSkyBoxRenderSystem::CreateRenderPass()
{
    // todo: 考虑在defer Render框架下，skybox作为defer Render pass的subpass实现
     // 深度附件描述
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_swapChain->FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;        // 设定多重采样(1表示不采样)
    // depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;           // 渲染通道开始时对深度附件数据的加载操作(这里表示开始时清空附件)
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;     // !渲染通道结束时对深度附件数据的加载操作(这里表示不关心)
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;           // 渲染通道开始时对深度附件数据的加载操作(保留模型渲染的结果)

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
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;        // !加载颜色附件的数据，不要清理掉了
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     // 渲染通道结束时保存颜色数据,以便后续呈现
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // !colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // 表示该附件在渲染结束后用于呈现
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 表示该附件用于shader的读入（喂给后处理）

    // 颜色附件引用
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;  // 表示第一个附件
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   // 表示用于颜色附件的最佳布局

    // 子通道描述:定义一个子通道，子通道是渲染通道中的一个渲染步骤。
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;    // 指定绑定的管线类型(这里表示绑定图形管线)
    subpass.colorAttachmentCount = 1;       // 颜色附件数量
    subpass.pColorAttachments = &colorAttachmentRef;    // 颜色附件引用的指针
    subpass.pDepthStencilAttachment = &depthAttachmentRef;  // 深度模板附件引用的指针

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

void VulkanSkyBoxRenderSystem::CreateCubeImage()
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
    imageInfo.arrayLayers = 6;  // cubemap为六面
    imageInfo.format = m_swapChain->GetSwapChainImageFormat();
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;   // 只是用作sample
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    VulkanGraphicsDevice::GetGraphicsDevice()->CreateImageWithInfo(imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = m_swapChain->GetSwapChainImageFormat();
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;       // cubemap为6层
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image views");
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

void VulkanSkyBoxRenderSystem::CreateDescriptorSet(const VulkanSkyBoxCreateInfo& createInfo)
{
    m_descriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSet newTextureSet;
        VulkanDescriptorWriter(*m_descriptorSetLayout[0], *createInfo.descriptorPool)
        .WriteImage(0, &m_imageInfo)
        .Build(newTextureSet);
        m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::Texture, newTextureSet);
    }
}

#endif

