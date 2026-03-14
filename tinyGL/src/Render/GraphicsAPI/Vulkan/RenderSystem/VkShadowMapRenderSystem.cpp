#include "VkShadowMapRenderSystem.h"

#include "VkModelRenderSystem.hpp"
#ifdef RENDER_IN_VULKAN
#include "Actor.hpp"
#include "Scene.hpp"
#include "Component/LightComponent.h"
#include "Render/RenderModule.hpp"
#include "Render/Resource/Texture.hpp"

using namespace Kong;

VkShadowMapRenderSystem::VkShadowMapRenderSystem(const VulkanShadowMapCreateInfo &createInfo)
{
    m_directLightShadowMapRenderSystem = make_unique<VkDirectLightShadowMapRenderSystem>(createInfo);
}

VkShadowMapRenderSystem::~VkShadowMapRenderSystem()
{
}

void VkShadowMapRenderSystem::InitLightShadowMapResource(VulkanDescriptorPool* descriptorPool)
{
    m_directLightShadowMapRenderSystem->InitLightShadowMapResource(descriptorPool);
}

void VkShadowMapRenderSystem::Draw(const FrameInfo& frameInfo)
{
    m_directLightShadowMapRenderSystem->Draw(frameInfo);
}

VkImageView VkShadowMapRenderSystem::GetShadowMapDebugImageView()
{
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        if (auto light_component = actor->GetComponent<CDirectionalLightComponent>())
        {
            return light_component->m_depthTexture->m_imageView;
        }
    }
    return VK_NULL_HANDLE;
}

VkSampler VkShadowMapRenderSystem::GetShadowMapDebugSampler()
{
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        if (auto light_component = actor->GetComponent<CDirectionalLightComponent>())
        {
            return light_component->m_depthTexture->m_sampler;
        }
    }
    return VK_NULL_HANDLE;
}

VkDirectLightShadowMapRenderSystem::VkDirectLightShadowMapRenderSystem(
    const VulkanShadowMapCreateInfo& createInfo)
{
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();

    // 因为只有一个depth stencil纹理，所以clearValues[0]就需要清理depthStencil
    m_clearValues[0].color = {0.0, 0.0, 0.0, 0.0};
    m_clearValues[0].depthStencil = {1.0, 0};
    //InitLightShadowMapResource(createInfo.descriptorPool);
    renderAreaExtent = {SHADOW_RESOLUTION, SHADOW_RESOLUTION};
}

VkDirectLightShadowMapRenderSystem::~VkDirectLightShadowMapRenderSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(device, m_renderPass, nullptr);
}

void VkDirectLightShadowMapRenderSystem::Draw(const FrameInfo& frameInfo)
{
    
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        if (auto light_component = actor->GetComponent<CDirectionalLightComponent>())
        {
            BeginRenderPass(frameInfo.commandBuffer, light_component->GetFrameBuffer());
            m_pipeline->Bind(frameInfo.commandBuffer);
            light_component->RenderShadowMap(frameInfo, m_pipelineLayout);
            EndRenderPass(frameInfo.commandBuffer);
            light_component->ConvertDepthTextureLayout(frameInfo);
        }
    }
}

void VkDirectLightShadowMapRenderSystem::CreateRenderPass()
{
     // 深度附件描述
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_swapChain->FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;        // 设定多重采样(1表示不采样)
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;       // 渲染通道开始时对深度附件数据的加载操作(这里表示开始时清空附件)
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     // 渲染通道结束时对深度附件数据的保存操作

    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;    // 渲染通道开始和结束时对模板缓冲区的操作
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // 指定附件在渲染通道开始时的图像布局,这里需不能用UNDEFINED因为是加载并复用了模型渲染的数据
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;    
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // 指定附件在渲染通道结束时的图像布局(这里表示用于深度模板附近的最佳布局)

    // 深度附件引用:定义子通道中如何引用深度附件
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 0;      // !指定附件的索引, 表示是第一个附件(从0开始)
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;       // 指定子通道中使用附件时的图像布局(这里表示用于深度附件的最佳布局)

    // shadow map不包括颜色附件
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;    // 指定绑定的管线类型(这里表示绑定图形管线)
    subpass.colorAttachmentCount = 0;       // 颜色附件数量
    subpass.pColorAttachments = VK_NULL_HANDLE;    // 颜色附件引用的指针
    subpass.pDepthStencilAttachment = &depthAttachmentRef;  // 深度模板附件引用的指针

    // 子通道依赖:定义子通道之间的依赖关系，确保渲染操作按正确顺序执行。
    // !这些要和lightcomponent中深度贴图经过barrier布局转换的access mask对应上
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;    // 指定源子通道(这里表示渲染通道外部的操作)
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;   // 指定源子通道的访问掩码
    dependency.srcStageMask =  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 指定管线阶段(这里表示包括早期片段测试阶段)
    
    dependency.dstSubpass = 0;  // 指定目标子通道,这里表示当前子通道
    dependency.dstStageMask= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;   // 指定目标子通道的管线阶段
    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // 指定目标子通道访问掩码,包括颜色附件写入和深度模板附件写入操作
    dependency.dependencyFlags = 0;

    // 创建渲染通道
    VkAttachmentDescription attachments[] = {depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass");
    }
}

void VkDirectLightShadowMapRenderSystem::CreateDescriptorSetLayout()
{
    // light project view matrix, 只有vertex shader需要
    auto basicInfoLayout = VulkanDescriptorSetLayout::Builder()
    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
    .Build();
    m_descriptorSetLayout.push_back(std::move(basicInfoLayout));
}

void VkDirectLightShadowMapRenderSystem::InitLightShadowMapResource(VulkanDescriptorPool* descriptorPool)
{
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        if (auto light_component = actor->GetComponent<CDirectionalLightComponent>())
        {
            light_component->InitShadowMap(m_renderPass, descriptorPool, m_descriptorSetLayout);
        }
    }
}

void VkDirectLightShadowMapRenderSystem::CreatePipeline()
{
    assert(m_pipelineLayout != nullptr && "pipelineLayout is null");
    
    PipelineConfigInfo pipelineConfig{};
    VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
        
    pipelineConfig.renderPass = m_renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    // pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {vs, "shader/Vulkan/shadow/sm_dir_light.vulkan.vert.spv"},
        {fs, "shader/Vulkan/shadow/sm_dir_light.vulkan.frag.spv"}},
        pipelineConfig);
}

void VkDirectLightShadowMapRenderSystem::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(VkModelRenderSystem::SimplePushConstantData);
    
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    // descriptor set layout
    descriptorSetLayouts.push_back(KongRenderModule::GetRenderModule().m_descriptorLayout->GetDescriptorSetLayout());
   
    // for (auto& layout : m_descriptorSetLayout)
    // {
    //     descriptorSetLayouts.push_back(layout->GetDescriptorSetLayout());
    // }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    if (vkCreatePipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

#endif