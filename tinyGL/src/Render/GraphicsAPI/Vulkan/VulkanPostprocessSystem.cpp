#include "VulkanPostprocessSystem.hpp"

#include <array>

#include "VulkanSwapChain.hpp"
#include "Render/RenderModule.hpp"

#ifdef RENDER_IN_VULKAN
using namespace Kong;



VulkanPostprocessSystem::VulkanPostprocessSystem(VulkanSwapChain* swapchain, VulkanDescriptorPool* descriptorPool)
    : m_swapChain(swapchain)
{
    // CreateRenderPass();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
    CreateDescriptorBuffer();
    

    
    quadShape = make_unique<CQuadShape>();
    
    auto& render_sys = KongRenderModule::GetRenderModule().m_simpleRenderSystem;
    m_imageInfo = {
        render_sys->GetSampler(),
        render_sys->GetColorImageView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    m_descriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSet newTextureSet;
        VulkanDescriptorWriter(*m_descriptorSetLayout[0], *descriptorPool)
        .WriteImage(0, &m_imageInfo)
        .Build(newTextureSet);
        m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessTexture, newTextureSet);

        VkDescriptorSet newSet;
        auto bufferInfo = m_uniformBuffers[i]->DescriptorInfo();
        VulkanDescriptorWriter(*m_descriptorSetLayout[1], *descriptorPool)
        .WriteBuffer(0, &bufferInfo)
        .Build(newSet);
        m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessData, newSet);
    }
}

VulkanPostprocessSystem::~VulkanPostprocessSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    // vkDestroyRenderPass(device, m_renderPass, nullptr);
}

void VulkanPostprocessSystem::Draw(const FrameInfo& frameInfo)
{
    m_pipeline->Bind(frameInfo.commandBuffer);
    // todo: 用到的时候再更新
    VulkanPostprocessUbo ubo{};
    ubo.bloom = 1.0;
    ubo.exposure = 0.1f;
    m_uniformBuffers[frameInfo.frameIndex]->WriteToBuffer(&ubo);
    m_uniformBuffers[frameInfo.frameIndex]->Flush();
    
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout, 0, 1,
        &m_descriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessTexture], 0, nullptr
        );

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout, 1, 1,
        &m_descriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessData], 0, nullptr
    );
    
    quadShape->mesh_resource->mesh_list[0]->m_RenderInfo->Draw(frameInfo.commandBuffer);
}

void VulkanPostprocessSystem::CreateDescriptorSetLayout()
{
    auto textureLayout = VulkanDescriptorSetLayout::Builder()
    // image 贴图数据
    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // scene image
    .Build();
    textureLayout->m_usage = VulkanDescriptorSetLayout::PostProcessTexture;
    m_descriptorSetLayout.push_back(std::move(textureLayout));
    
    auto dataLayout = VulkanDescriptorSetLayout::Builder()
    // 后处理数据
    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // scene image
    .Build();
    dataLayout->m_usage = VulkanDescriptorSetLayout::PostProcessData;
    m_descriptorSetLayout.push_back(std::move(dataLayout));
}

void VulkanPostprocessSystem::CreatePipelineLayout()
{
    // set按顺序存在vector中，set0,set1,set2 ...
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    for (int i = 0; i < m_descriptorSetLayout.size(); i++)
    {
        descriptorSetLayouts.push_back(m_descriptorSetLayout[i]->GetDescriptorSetLayout());
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void VulkanPostprocessSystem::CreatePipeline()
{
    assert(m_pipelineLayout != nullptr && "pipelineLayout is null");
    
    // 使用swapchain的大小而不是Windows的，因为这两个有可能不是一一对应
    PipelineConfigInfo pipelineConfig{};
    VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = m_swapChain->GetRenderPass();   // 后处理用swapchain的renderpass
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {vs, "shader/Vulkan/simple_postprocess.vulkan.vert.spv"},
        {fs, "shader/Vulkan/simple_postprocess.vulkan.frag.spv"}},
        pipelineConfig);
}

void VulkanPostprocessSystem::CreateRenderPass()
{
     // 深度附件描述
    // VkAttachmentDescription depthAttachment = {};
    // depthAttachment.format = m_swapChain->FindDepthFormat();
    // depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;        // 设定多重采样(1表示不采样)
    // depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;           // 渲染通道开始时对深度附件数据的加载操作(这里表示开始时清空附件)
    // depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;     // 渲染通道结束时对深度附件数据的加载操作(这里表示不关心)
    // depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;    // 渲染通道开始和结束时对模板缓冲区的操作
    // depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;          // 指定附件在渲染通道开始时的图像布局(这里表示不关心)
    // depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // 指定附件在渲染通道结束时的图像布局(这里表示用于深度模板附近的最佳布局)
    //
    // // 深度附件引用:定义子通道中如何引用深度附件
    // VkAttachmentReference depthAttachmentRef = {};
    // depthAttachmentRef.attachment = 1;      // 指定附件的索引, 表示是第二个附件(从0开始)
    // depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;       // 指定子通道中使用附件时的图像布局(这里表示用于深度附件的最佳布局)
    //
    // // 颜色附件描述
    // VkAttachmentDescription colorAttachment = {};
    // colorAttachment.format = m_swapChain->GetSwapChainImageFormat();
    // colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     // 渲染通道结束时保存颜色数据,以便后续呈现
    // colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // 表示该附件在渲染结束后用于呈现
    //
    // // 颜色附件引用
    // VkAttachmentReference colorAttachmentRef = {};
    // colorAttachmentRef.attachment = 0;  // 表示第一个附件
    // colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   // 表示用于颜色附件的最佳布局
    //
    // /*
    //  * 一个渲染通道可以包含多个子通道，每个子通道定义了一组特定的渲染操作，比如绘制几何体、进行后处理等。
    //  * 子通道允许在不切换渲染目标的情况下，对同一组附件执行多个渲染操作，从而减少不必要的内存操作和同步开销。
    //  */
    // // 子通道描述:定义一个子通道，子通道是渲染通道中的一个渲染步骤。
    // VkSubpassDescription subpass = {};
    // subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;    // 指定绑定的管线类型(这里表示绑定图形管线)
    // subpass.colorAttachmentCount = 1;       // 颜色附件数量
    // subpass.pColorAttachments = &colorAttachmentRef;    // 颜色附件引用的指针
    // subpass.pDepthStencilAttachment = &depthAttachmentRef;  // 深度模板附件引用的指针
    //
    // /*
    //  * 子通道依赖用于描述子通道之间的执行顺序和数据依赖关系。
    //  * 它确保一个子通道在另一个子通道完成特定操作后才开始执行，从而保证渲染结果的正确性。
    //  */
    // // 子通道依赖:定义子通道之间的依赖关系，确保渲染操作按正确顺序执行。
    // VkSubpassDependency dependency = {};
    // dependency.srcSubpass = VK_SUBPASS_EXTERNAL;    // 指定源子通道(这里表示渲染通道外部的操作)
    // dependency.srcAccessMask = 0;   // 指定源子通道的访问掩码(这里表示不关心)
    // dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    // | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 指定管线阶段(这里表示包括颜色附件输出阶段和早期片段测试阶段)
    //
    // dependency.dstSubpass = 0;  // 指定目标子通道,这里表示当前子通道
    // dependency.dstStageMask= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    // | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 指定目标子通道的管线阶段,和源子通道相同
    // dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    // | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // 指定目标子通道访问掩码,包括颜色附件写入和深度模板附件写入操作
    //
    // // 创建渲染通道
    // std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    // VkRenderPassCreateInfo renderPassInfo = {};
    // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // renderPassInfo.attachmentCount = attachments.size();
    // renderPassInfo.pAttachments = attachments.data();
    // renderPassInfo.subpassCount = 1;
    // renderPassInfo.pSubpasses = &subpass;
    // renderPassInfo.dependencyCount = 1;
    // renderPassInfo.pDependencies = &dependency;
    //
    // if (vkCreateRenderPass(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to create render pass");
    // }
}

void VulkanPostprocessSystem::CreateDescriptorBuffer()
{
    m_uniformBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < m_uniformBuffers.size(); ++i)
    {
        m_uniformBuffers[i] = std::make_unique<VulkanBuffer>();
        m_uniformBuffers[i]->Initialize(UNIFORM_BUFFER, sizeof(BasicMaterialUbo), 1);
        m_uniformBuffers[i]->Map();
    }
}

#endif
