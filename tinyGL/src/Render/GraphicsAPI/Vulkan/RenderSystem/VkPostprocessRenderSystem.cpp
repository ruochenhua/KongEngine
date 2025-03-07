
#include "VkPostprocessRenderSystem.hpp"

#include <array>

#include "Render/RenderModule.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanPipeline.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"

#ifdef RENDER_IN_VULKAN
using namespace Kong;

VulkanPostprocessSystem::VulkanPostprocessSystem(const VulkanPostprocessCreateInfo &createInfo)
    : VulkanRenderSystem(createInfo.swapChain)
{
    // CreateRenderPass();
    m_renderPass = m_swapChain->GetRenderPass();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
    CreateDescriptorBuffer();
    CreateDescriptorSet(createInfo);
    
    quadShape = make_unique<CQuadShape>();
}

VulkanPostprocessSystem::~VulkanPostprocessSystem()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    // vkDestroyRenderPass(device, m_renderPass, nullptr);
}

void VulkanPostprocessSystem::Draw(const FrameInfo& frameInfo)
{
    // 用swapchain的framebuffer
    m_framebuffer = m_swapChain->GetFrameBuffer(frameInfo.frameIndex);
    
    BeginRenderPass(frameInfo.commandBuffer);
    
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

    if (auto quadMesh = quadShape->GetMesh())
    {
        quadMesh->m_RenderInfo->Draw(frameInfo.commandBuffer);
    }
    
    EndRenderPass(frameInfo.commandBuffer);
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
    pipelineConfig.renderPass = m_renderPass;   // 后处理用swapchain的renderpass
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {vs, "shader/Vulkan/simple_postprocess.vulkan.vert.spv"},
        {fs, "shader/Vulkan/simple_postprocess.vulkan.frag.spv"}},
        pipelineConfig);
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

void VulkanPostprocessSystem::CreateDescriptorSet(const VulkanPostprocessCreateInfo &createInfo)
{
    m_imageInfo = {
        createInfo.sampler,
        createInfo.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    
    m_descriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSet newTextureSet;
        VulkanDescriptorWriter(*m_descriptorSetLayout[0], *createInfo.descriptorPool)
        .WriteImage(0, &m_imageInfo)
        .Build(newTextureSet);
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
