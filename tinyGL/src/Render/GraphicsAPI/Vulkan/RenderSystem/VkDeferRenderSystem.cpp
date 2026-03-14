#include "VkDeferRenderSystem.hpp"

#include <array>

#include "Actor.hpp"
#include "Scene.hpp"
#include "Component/LightComponent.h"
#include "Render/RenderModule.hpp"
#include "Render/Resource/Texture.hpp"

using namespace Kong;
#include "VkDeferRenderSystem.hpp"

#include <array>

#include "Actor.hpp"
#include "Scene.hpp"
#include "Component/LightComponent.h"
#include "Render/RenderModule.hpp"
#include "Render/Resource/Texture.hpp"

using namespace Kong;

#ifdef RENDER_IN_VULKAN

VkDeferRenderSystem::VkDeferRenderSystem()
{
    CreateDeferGeometryTexture();
    CreateDeferColorDescriptorSetLayout();
    CreateDeferColorPipelineLayout();
    CreateRenderPass();
    CreateFrameBuffers();
    CreatePipeline();
    CreateDescriptorSets();
    
    // framebuffer和render pass对应的attachment0-4=color；attachment5=depth
    // depthStencil clear的值为1,0
    m_clearValues.resize(6);
    m_clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_clearValues[3].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_clearValues[4].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_clearValues[5].depthStencil = { 1.0f, 0 };
    m_quadShape = make_unique<CQuadShape>();
}

VkDeferRenderSystem::~VkDeferRenderSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();

    vkDestroyRenderPass(device, m_renderPass, nullptr);
    vkDestroyFramebuffer(device, m_framebuffer, nullptr);

    vkDestroyPipelineLayout(device, m_deferColorPipelineLayout, nullptr);
}

bool initShadow = false;
void VkDeferRenderSystem::Draw(const FrameInfo& frameInfo)
{
    // test shadow code block
    if (!initShadow)
    {
        auto actors = KongSceneManager::GetActors();
        for (auto actor : actors)
        {
            if (auto light_component = actor->GetComponent<CDirectionalLightComponent>())
            {
                VkDescriptorImageInfo shadowImageInfo {};
                shadowImageInfo.sampler = light_component->m_depthTexture->m_sampler;
                shadowImageInfo.imageView = light_component->m_depthTexture->m_imageView;
                shadowImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
                for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
                {
                    VkWriteDescriptorSet shadowWriteInfo {};
                    shadowWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    shadowWriteInfo.dstBinding = 4;
                    shadowWriteInfo.descriptorCount = 1;
                    shadowWriteInfo.pImageInfo = &shadowImageInfo;
                    shadowWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
                    // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER需要显式提供采样器
                    shadowWriteInfo.dstSet = m_deferColorDescriptorSets[i];
                    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
                    vkUpdateDescriptorSets(device, 1, &shadowWriteInfo, 0, nullptr);
                }
                initShadow = true;
            }
        }
    }
    
    BeginRenderPass(frameInfo.commandBuffer);
    
    m_pipeline->Bind(frameInfo.commandBuffer);
    // geometry phase draw
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto mesh_component = actor->GetComponent<CMeshComponent>();
        if (!mesh_component)
        {
            continue;
        }

        auto mesh_shader = mesh_component->shader_data;
        SimplePushConstantData push{};
        push.modelMatrix = actor->GetModelMatrix();

        vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(SimplePushConstantData), &push);
        
        mesh_component->Draw(frameInfo, m_pipelineLayout);
    }
    
    
    // 启动下一个subpass
    vkCmdNextSubpass(frameInfo.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
    
    // 光照阶段管线
    m_deferColorPipeline->Bind(frameInfo.commandBuffer);
    
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_deferColorPipelineLayout,
        0, 1,
        &KongRenderModule::GetRenderModule().m_descriptorSets[frameInfo.frameIndex]
            , 0, nullptr);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_deferColorPipelineLayout,
        1, 1,
        &m_deferColorDescriptorSets[frameInfo.frameIndex]
            , 0, nullptr);
            
            
    // lighting phase draw
    if (auto quadMesh = m_quadShape->GetMesh())
    {
        quadMesh->m_RenderInfo->Draw(frameInfo.commandBuffer);
    }
    
    EndRenderPass(frameInfo.commandBuffer);
}

void VkDeferRenderSystem::CreatePipeline()
{
    assert(m_pipelineLayout != nullptr && "pipelineLayout is null");
    {
        PipelineConfigInfo pipelineConfig{};
        VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(4);
        for (auto& attachment : colorBlendAttachments)
        {
            attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;    // �������ص�ʱ������Щɫֵ
            attachment.blendEnable = VK_FALSE;     //glEnable(GL_BLEND)
        }

        pipelineConfig.colorBlendAttachment = colorBlendAttachments;
        pipelineConfig.colorBlendInfo.attachmentCount = colorBlendAttachments.size();
        pipelineConfig.colorBlendInfo.pAttachments = colorBlendAttachments.data();
        
        pipelineConfig.renderPass = m_renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        pipelineConfig.subpass = 0;
        m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
            {vs, "shader/Vulkan/defer/defer_geo.vulkan.vert.spv"},
            {fs, "shader/Vulkan/defer/defer_geo.vulkan.frag.spv"}},
            pipelineConfig);
    }

    {
        PipelineConfigInfo pipelineConfig{};
        VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_renderPass;
        pipelineConfig.pipelineLayout = m_deferColorPipelineLayout;
        pipelineConfig.subpass = 1;
        // !光照阶段修改深度附件，否则后面的步骤会出错
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        m_deferColorPipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
            {vs, "shader/Vulkan/defer/defer_lighting.vulkan.vert.spv"},
            {fs, "shader/Vulkan/defer/defer_lighting.vulkan.frag.spv"}},
            pipelineConfig);
    }
}

void VkDeferRenderSystem::CreateRenderPass()
{
    std::vector<VkAttachmentDescription> geoStageOutputAttachments = {};
    // ���ν׶ε����
    {
        // 0λ��
        VkAttachmentDescription positionAttachment = {};
        positionAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // ��Ⱦͨ������ʱ������ɫ����,�Ա��������
        positionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        positionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        positionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        positionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;  // ����ɫ�׶ε�shader��ȡ
        geoStageOutputAttachments.push_back(positionAttachment);
        
        // 1����
        VkAttachmentDescription normalAttachment = {};
        normalAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        normalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        geoStageOutputAttachments.push_back(normalAttachment);
        
        // 2��ɫ
        VkAttachmentDescription albedoAttachment = {};
        albedoAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        albedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        albedoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        albedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        albedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        albedoAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        geoStageOutputAttachments.push_back(albedoAttachment);
        
        // 3����
        VkAttachmentDescription ormAttachment = {};
        ormAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        ormAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        ormAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        ormAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        ormAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        ormAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        ormAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ormAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        geoStageOutputAttachments.push_back(ormAttachment);

        // 4���ս׶�����ĸ���
        VkAttachmentDescription outputColorAttachment = {};
#if USE_COMPUTE_POSTPROCESS
        outputColorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
#else
        outputColorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
#endif
        
        outputColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        outputColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        outputColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        outputColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        outputColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        outputColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        outputColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        geoStageOutputAttachments.push_back(outputColorAttachment);
    }

    // 5��ȸ�������
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_swapChain->FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;        
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   
    // depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     
    
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    geoStageOutputAttachments.push_back(depthAttachment);
    
    // ���ν׶���ɫ��������
    std::vector<VkAttachmentReference> geoStageAttachmentRefs = {};
    for (int i = 0; i < geoStageOutputAttachments.size()-1; i++)    
    {
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = static_cast<uint32_t>(i);  
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  
        geoStageAttachmentRefs.push_back(colorAttachmentRef);
    }

    // ��ȸ�������
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = static_cast<uint32_t>(geoStageOutputAttachments.size()-1); //depth位置在最后
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    /*
     * 子通道依赖用于描述子通道之间的执行顺序和数据依赖关系。
     * 它确保一个子通道在另一个子通道完成特定操作后才开始执行，从而保证渲染结果的正确性。
     */
    // 子通道依赖:定义子通道之间的依赖关系，确保渲染操作按正确顺序执行。
    std::vector<VkSubpassDescription> subpasses(2);
    // 两个subpass
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;// 指定绑定的管线类型(这里表示绑定图形管线)
    subpasses[0].colorAttachmentCount = static_cast<uint32_t>(geoStageAttachmentRefs.size()-1); // 颜色附件数量
    subpasses[0].pColorAttachments = geoStageAttachmentRefs.data(); // 颜色附件引用的指针
    subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;// 深度模板附件引用的指针

    // 几何阶段作为光照阶段输入的attachment refs
    std::vector<VkAttachmentReference> inputAttachmentRefs;
    for (int i = 0; i < geoStageOutputAttachments.size()-2; i++)
    {
        VkAttachmentReference inputAttachmentRef = {};
        inputAttachmentRef.attachment = static_cast<uint32_t>(i);
        inputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        inputAttachmentRefs.push_back(inputAttachmentRef);
    }
    
    VkAttachmentReference outputAttachmentRef = {};
    outputAttachmentRef.attachment = geoStageOutputAttachments.size() - 2;  
    outputAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[1].inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRefs.size());
    subpasses[1].pInputAttachments = inputAttachmentRefs.data();
    subpasses[1].colorAttachmentCount = 1;
    subpasses[1].pColorAttachments = &outputAttachmentRef;
    // !光照阶段不需要传入depth，不需要深度附件否则后面的步骤会出错
    subpasses[1].pDepthStencilAttachment = &depthAttachmentRef;
    
    /*
     * 子通道依赖用于描述子通道之间的执行顺序和数据依赖关系。
     * 它确保一个子通道在另一个子通道完成特定操作后才开始执行，从而保证渲染结果的正确性。
     */
    // 子通道依赖:定义子通道之间的依赖关系，确保渲染操作按正确顺序执行。
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = 0;   
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;   // 指定源子通道的访问掩码(这里表示不关心)
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 指定管线阶段(这里表示包括颜色附件输出阶段和早期片段测试阶段)
    
    dependency.dstSubpass = 1;  // 指定目标子通道
    dependency.dstStageMask= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
    | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 指定目标子通道的管线阶段,和源子通道相同
    dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT
    | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // 指定目标子通道访问掩码,包括颜色附件写入和深度模板附件写入操作
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // VK_DEPENDENCY_BY_REGION_BIT表示存在subpass前后的依赖关系
    
    std::vector<VkAttachmentDescription> attachments = {geoStageOutputAttachments};
    
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass");
    }
}

void VkDeferRenderSystem::CreateDescriptorSets()
{
    
    std::vector<VkImageView> imageViews = {
        m_positionTexture->m_imageView,
        m_normalTexture->m_imageView,
        m_albedoTexture->m_imageView,
        m_ormTexture->m_imageView,
    };
    
    std::vector<VkDescriptorImageInfo> imageInfos(imageViews.size());
    for (int i = 0; i < imageViews.size(); i++)
    {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = imageViews[i];
    }

    
    auto nullTex = dynamic_cast<VulkanTexture*>(KongRenderModule::GetNullTex());
    VkDescriptorImageInfo shadowImageInfo {nullTex->m_sampler, nullTex->m_imageView,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    
    auto descriptorPool = KongRenderModule::GetRenderModule().m_descriptorPool.get();
    m_deferColorDescriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSet newTextureSet;
        VulkanDescriptorWriter(*m_deferColorDescriptorSetLayouts[0], *descriptorPool)
        .WriteImage(0, &imageInfos[0])
        .WriteImage(1, &imageInfos[1])
        .WriteImage(2, &imageInfos[2])
        .WriteImage(3, &imageInfos[3])
        .WriteImage(4, &shadowImageInfo)
        .Build(m_deferColorDescriptorSets[i]);
    }
}

void VkDeferRenderSystem::CreateFrameBuffers()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    auto extent = m_swapChain->GetSwapChainExtent();
    
    VkFramebufferCreateInfo framebufferInfo = {};
    // std::array<VkImageView, 2> attachments = {m_sceneTexture->m_imageView, m_depthTexture->m_imageView};
    std::array<VkImageView, 6> attachments = {
        m_positionTexture->m_imageView,
        m_normalTexture->m_imageView,
        m_albedoTexture->m_imageView,
        m_ormTexture->m_imageView,
        m_sceneTexture->m_imageView,
        m_depthTexture->m_imageView
    };
    
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

void VkDeferRenderSystem::CreateDeferColorDescriptorSetLayout()
{
    auto textureLayout = VulkanDescriptorSetLayout::Builder()
    // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    .AddBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // position texture
    .AddBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // normal texture
    .AddBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // albedo texture
    .AddBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // orm texture
    .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // direct shadow map   
    .Build();

    textureLayout->m_usage = VulkanDescriptorSetLayout::Texture;
    m_deferColorDescriptorSetLayouts.push_back(std::move(textureLayout));
}

void VkDeferRenderSystem::CreateDeferColorPipelineLayout()
{
    // set��˳�����vector�У�set0,set1,set2 ...
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    // �ȷ�ȫ�ֵ�descriptor set layout
    descriptorSetLayouts.push_back(KongRenderModule::GetRenderModule().m_descriptorLayout->GetDescriptorSetLayout());
    for (auto& layout : m_deferColorDescriptorSetLayouts)
    {
        descriptorSetLayouts.push_back(layout->GetDescriptorSetLayout());
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    
    if (vkCreatePipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(),
        &pipelineLayoutInfo, nullptr, &m_deferColorPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void VkDeferRenderSystem::CreateDeferGeometryTexture()
{
    VkDevice device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    VkExtent2D extent = m_swapChain->GetSwapChainExtent();
    VkFormat imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
    
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    
    m_positionTexture = make_unique<VulkanTexture>(imageInfo);
    m_positionTexture->CreateImageView(imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    m_positionTexture->CreateTextureSampler();

    m_normalTexture = make_unique<VulkanTexture>(imageInfo);
    m_normalTexture->CreateImageView(imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    m_normalTexture->CreateTextureSampler();

    m_albedoTexture = make_unique<VulkanTexture>(imageInfo);
    m_albedoTexture->CreateImageView(imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    m_albedoTexture->CreateTextureSampler();

    m_ormTexture = make_unique<VulkanTexture>(imageInfo);
    m_ormTexture->CreateImageView(imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    m_ormTexture->CreateTextureSampler();
    
}

#endif
