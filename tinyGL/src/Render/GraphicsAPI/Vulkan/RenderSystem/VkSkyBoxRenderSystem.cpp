#include "VkSkyBoxRenderSystem.hpp"

#include <array>

#include "Scene.hpp"
#include "Render/RenderModule.hpp"

using namespace Kong;
#ifdef RENDER_IN_VULKAN
VulkanSkyBoxRenderSystem::VulkanSkyBoxRenderSystem(const VulkanSkyBoxCreateInfo& createInfo)
{
    // 可能是simple Render，可能是defer Render
    m_boxShape = make_unique<CBoxShape>();

    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
    CreateCubeImage();
    CreateDescriptorSet(createInfo);
    // CreateTextures();
    CreateFramebuffer(createInfo);
}

VulkanSkyBoxRenderSystem::~VulkanSkyBoxRenderSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(device, m_renderPass, nullptr);
    vkDestroyFramebuffer(device, m_framebuffer, nullptr);
}

void VulkanSkyBoxRenderSystem::Draw(const FrameInfo& frameInfo)
{
    SetBarrier(frameInfo.commandBuffer);
    BeginRenderPass(frameInfo.commandBuffer);

    m_pipeline->Bind(frameInfo.commandBuffer);
    // 全局变量，控制在render module中
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0, 1,
        &KongRenderModule::GetRenderModule().m_descriptorSets[frameInfo.frameIndex]
            , 0, nullptr);
    
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout, 1, 1,
        &m_descriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::Texture],
        0, nullptr);

    m_boxShape->Draw(frameInfo.commandBuffer);
    
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
    // !这里关掉深度写入，深度测试保持开启
    // pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
    pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    // !开启前面剔除，等于glCullFace(GL_FRONT)这一步
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;   // 用逆时针这一面作为front
    // * 比较方法这里使用less_or_equal，天空盒深度为1.0的时候才不会出现z fighting
    pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        
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
    // 指定附件在渲染通道开始时的图像布局,这里需不能用UNDEFINED因为是加载并复用了模型渲染的数据
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;    
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
    // 指定附件在渲染通道开始时的图像布局,这里需不能用UNDEFINED因为是加载并复用了模型渲染的数据
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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
    /**
    立方体贴图的面是有顺序的。面索引对应如下关系：
        0：+X（左）
        1：-X（右）
        2：+Y（上）
        3：-Y（下）
        4：+Z（前）
        5：-Z（后）
     */
    m_cubeMap = dynamic_pointer_cast<VulkanTexture>(ResourceManager::GetOrLoadCubeTexture(diffuse, {
        
        CSceneLoader::ToResourcePath("/sky_box/dark_sky/darkskies_lf.tga"),
        CSceneLoader::ToResourcePath("/sky_box/dark_sky/darkskies_rt.tga"),
        CSceneLoader::ToResourcePath("/sky_box/dark_sky/darkskies_up.tga"),
        CSceneLoader::ToResourcePath("/sky_box/dark_sky/darkskies_dn.tga"),
        CSceneLoader::ToResourcePath("/sky_box/dark_sky/darkskies_ft.tga"),
        CSceneLoader::ToResourcePath("/sky_box/dark_sky/darkskies_bk.tga"),
    }));

    m_imageInfo = {m_cubeMap->m_sampler, m_cubeMap->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
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

void VulkanSkyBoxRenderSystem::CreateFramebuffer(const VulkanSkyBoxCreateInfo& createInfo)
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    auto extent = m_swapChain->GetSwapChainExtent();
    
    VkFramebufferCreateInfo framebufferInfo = {};
    std::array<VkImageView, 2> attachments = {
        createInfo.inputSceneTexture->m_imageView,
        createInfo.inputDepthTexture->m_imageView
    };

    m_inputSceneTexture = createInfo.inputSceneTexture;
    m_inputDepthTexture = createInfo.inputDepthTexture;
    
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
}

//* 设置 barrier
void VulkanSkyBoxRenderSystem::SetBarrier(VkCommandBuffer commandBuffer)
{
    // 进入天空盒渲染之前，需要设置图像的 barrier，以保证延迟渲染的结果正确输出到image上后再进行天空盒渲染
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 延迟渲染阶段的输出图像使用的布局
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 天空盒渲染阶段所需的布局
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // 当使用单队列时可忽略
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // 同上
    barrier.image = m_inputSceneTexture->m_image; // 延迟渲染的输出图像
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // 需要与上一步的写入相关的记得清理
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // 之后需要作为采样器读取

    // 手动指定用何种图形 API 提供的同步亮点使用
    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // 上一个阶段的管道
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // 当前阶段的管道
        0, // 标志
        0, nullptr,  // 依赖于 srcAccessMask 或 dstAccessMask 设置的内存屏障
        0, nullptr,  // 数组的内存屏障
        1, &barrier  // 图像内存屏障
    );
    
    // 再设置深度图像的 barrier
    VkImageMemoryBarrier depthBarrier = {};
    depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    depthBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // 上一阶段的布局
    depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // 为读取新的布局
    depthBarrier.image = m_inputDepthTexture->m_image; // 深度图像
    depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // 深度 aspect
    depthBarrier.subresourceRange.baseMipLevel = 0;
    depthBarrier.subresourceRange.levelCount = 1;
    depthBarrier.subresourceRange.baseArrayLayer = 0;
    depthBarrier.subresourceRange.layerCount = 1;
    depthBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // 上个阶段的写入操作
    depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; // 当前阶段的读取操作

    // 再次设置 barrier
    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // 上一个阶段
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, // 当前目标阶段
        0, // 标志
        0, nullptr,  // srcAccessMask 和 dstAccessMask
        0, nullptr,  // 数组的内存屏障
        1, &depthBarrier  // 深度图像内存屏障
    );
}

#endif

