
#include "VkPostprocessRenderSystem.hpp"

#include <imgui_impl_vulkan.h>

#include "Actor.hpp"
#include "Component/LightComponent.h"
#include "Render/RenderModule.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanPipeline.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"

#define SHADOWMAP_DEBUG 0

#ifdef RENDER_IN_VULKAN
using namespace Kong;


VulkanPostprocessSystem::VulkanPostprocessSystem(const VulkanPostprocessCreateInfo &createInfo)
{
#if USE_COMPUTE_POSTPROCESS
    // 创建存储图像
    CreateComputeResultTexture();
#endif
    // CreateRenderPass();
    m_renderPass = m_swapChain->GetRenderPass();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
    m_uniformBuffers = CreateDescriptorBuffer<VulkanPostprocessUbo>();
    CreateDescriptorSet(createInfo);
    
    quadShape = make_unique<CQuadShape>();
}

VulkanPostprocessSystem::~VulkanPostprocessSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

void VulkanPostprocessSystem::Draw(const FrameInfo& frameInfo)
{
#if SHADOWMAP_DEBUG
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
                    shadowWriteInfo.dstBinding = 1;
                    shadowWriteInfo.descriptorCount = 1;
                    shadowWriteInfo.pImageInfo = &shadowImageInfo;
                    shadowWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
                    // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER需要显式提供采样器
                    shadowWriteInfo.dstSet = m_descriptorSets[i][VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessTexture];
                    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
                    vkUpdateDescriptorSets(device, 1, &shadowWriteInfo, 0, nullptr);
                }
                initShadow = true;
            }
        }
    }
#endif
    
    // 用swapchain的framebuffer
    m_framebuffer = m_swapChain->GetFrameBuffer(frameInfo.frameIndex);
#if !USE_COMPUTE_POSTPROCESS
    BeginRenderPass(frameInfo.commandBuffer);
#endif
    
    m_pipeline->Bind(frameInfo.commandBuffer);
    
    // todo: 用到的时候再更新
    VulkanPostprocessUbo ubo{};
    ubo.bloom = 1.0;
    ubo.exposure = 0.1f;
    m_uniformBuffers[frameInfo.frameIndex]->WriteToBuffer(&ubo);
    m_uniformBuffers[frameInfo.frameIndex]->Flush();
    
#if USE_COMPUTE_POSTPROCESS
    // // 需要在这里转换scene texture的layout，然后更新compute shader输入
    // VkImageMemoryBarrier imageMemoryBarrier = {};
    // imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    // imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // imageMemoryBarrier.image = m_sceneImage;
    // imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    // imageMemoryBarrier.subresourceRange.levelCount = 1;
    // imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    // imageMemoryBarrier.subresourceRange.layerCount = 1;
    // imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //
    // // 调用 vkCmdPipelineBarrier
    // vkCmdPipelineBarrier(
    //     frameInfo.commandBuffer,
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    //     0,
    //     0, nullptr,
    //     0, nullptr,
    //     1, &imageMemoryBarrier
    // );

    // 更新输入image
    // for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    // {
    //VkDescriptorImageInfo newImageInfo {VK_NULL_HANDLE, m_sceneImage->view};
    VkWriteDescriptorSet sceneImageInfo {};
    sceneImageInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    sceneImageInfo.dstBinding = 0;
    sceneImageInfo.descriptorCount = 1;
    sceneImageInfo.pImageInfo = &m_imageInfo;
    sceneImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
    // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER需要显式提供采样器
    sceneImageInfo.dstSet = m_descriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessTexture];
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkUpdateDescriptorSets(device, 1, &sceneImageInfo, 0, nullptr);
    // }
    
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        m_pipelineLayout, 0, 1,
        &m_descriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessTexture], 0, nullptr
        );

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        m_pipelineLayout, 1, 1,
        &m_descriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessData], 0, nullptr
    );


    VkExtent2D extent = m_swapChain->GetSwapChainExtent();
    uint32_t gounpCountX = (extent.width + 15) / 16;
    uint32_t gounpCountY = (extent.height + 15) / 16;
    vkCmdDispatch(frameInfo.commandBuffer, gounpCountX, gounpCountY, 1);

// 将输出image layout转换
    
    
    BeginRenderPass(frameInfo.commandBuffer);
#else
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
    
    if (auto quadMesh = quadShape->GetMesh())
    {
        quadMesh->m_RenderInfo->Draw(frameInfo.commandBuffer);
    }
#endif

    // todo：暂时先放在这里
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
    EndRenderPass(frameInfo.commandBuffer);
}

void VulkanPostprocessSystem::CreateDescriptorSetLayout()
{
    auto textureLayout = VulkanDescriptorSetLayout::Builder()
    // image 贴图数据
#if USE_COMPUTE_POSTPROCESS
    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, 1) // scene image
    .AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1) // shadowmap测试
#else
    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // scene image
    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // shadowmap测试
#endif
    .Build();
    textureLayout->m_usage = VulkanDescriptorSetLayout::PostProcessTexture;
    m_descriptorSetLayout.push_back(std::move(textureLayout));
    
    auto dataLayout = VulkanDescriptorSetLayout::Builder()
    // 后处理数据
#if USE_COMPUTE_POSTPROCESS
    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1) // scene image
#else
    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // scene image
#endif
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
    pipelineConfig.renderPass = m_renderPass;   // 后处理用swapchain的renderpass
    pipelineConfig.pipelineLayout = m_pipelineLayout;
#if USE_COMPUTE_POSTPROCESS
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {cs, "shader/Vulkan/simple_postprocess.vulkan.comp.spv"},},
        pipelineConfig);
#else
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {vs, "shader/Vulkan/simple_postprocess.vulkan.vert.spv"},
        {fs, "shader/Vulkan/simple_postprocess.vulkan.frag.spv"}},
        pipelineConfig);
#endif
}

void VulkanPostprocessSystem::CreateComputeResultTexture()
{
    VkDevice device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    VkExtent2D extent {m_swapChain->GetSwapChainExtent()};

    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    
    m_csResultTexture = make_unique<VulkanTexture>(imageInfo);
    m_csResultTexture->CreateImageView(imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    m_csResultTexture->CreateTextureSampler();

    // 转换layout，从UNDEFINED转换成GENERAL
    m_csResultTexture->TransitionImageLayout(m_csResultTexture->m_image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void VulkanPostprocessSystem::CreateDescriptorSet(const VulkanPostprocessCreateInfo &createInfo)
{
    m_imageInfo = {
        createInfo.sampler,
        createInfo.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    m_sceneImage = createInfo.image;

    auto nullTex = dynamic_cast<VulkanTexture*>(KongRenderModule::GetNullTex());
    VkDescriptorImageInfo m_shadowImageInfo {nullTex->m_sampler, nullTex->m_imageView,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

#if USE_COMPUTE_POSTPROCESS
    // compute shader输出图像
    VkDescriptorImageInfo m_csOutputImageInfo { VK_NULL_HANDLE, m_csResultTexture->m_imageView,
        VK_IMAGE_LAYOUT_GENERAL
    };
#endif
    
    m_descriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSet newTextureSet;
#if !USE_COMPUTE_POSTPROCESS
        // compute shader需要转换layout
        VulkanDescriptorWriter(*m_descriptorSetLayout[0], *createInfo.descriptorPool)
        .WriteImage(0, &m_imageInfo)
        .WriteImage(1, &m_shadowImageInfo)
        .Build(newTextureSet);
#else
        VulkanDescriptorWriter(*m_descriptorSetLayout[0], *createInfo.descriptorPool)

        .WriteImage(1, &m_csOutputImageInfo)
        .Build(newTextureSet);
#endif
        m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessTexture, newTextureSet);

        VkDescriptorSet newSet;
        auto bufferInfo = m_uniformBuffers[i]->DescriptorInfo();
        VulkanDescriptorWriter(*m_descriptorSetLayout[1], *createInfo.descriptorPool)
        .WriteBuffer(0, &bufferInfo)
        .Build(newSet);
        m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::PostProcessData, newSet);
    }
}

#endif
