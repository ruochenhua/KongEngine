#include "SimpleVulkanRenderSystem.hpp"

#include "Scene.hpp"
#include "Render/RenderModule.hpp"

using namespace Kong;
#ifdef RENDER_IN_VULKAN

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.0f};
    alignas(16) glm::mat4 normalMatrix{1.0f};
};

SimpleVulkanRenderSystem::SimpleVulkanRenderSystem(VulkanGraphicsDevice& deviceRef, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : m_deviceRef(deviceRef)
{
    CreatePipelineLayout(globalSetLayout);
    CreatePipeline(renderPass);
}

SimpleVulkanRenderSystem::~SimpleVulkanRenderSystem()
{
    vkDestroyPipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), m_pipelineLayout, nullptr);
}

void SimpleVulkanRenderSystem::RenderGameObjects(const FrameInfo& frameInfo)
{
    m_pipeline->Bind(frameInfo.commandBuffer);
    // 绑定descriptor set
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
    

    auto projectionMat = KongRenderModule::GetRenderModule().GetCamera()->GetProjectionMatrix();
    auto viewMat = KongRenderModule::GetRenderModule().GetCamera()->GetViewMatrix();
    auto projectionView =  projectionMat * viewMat;
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        // todo: 简单渲染
    }
    // for (auto& object : gameObjects)
    // {
    //     SimplePushConstantData push{};
    //     push.modelMatrix = projectionView * object.transform.mat4();
    //     
    //     
    //     vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout,
    //         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    //         0, sizeof(SimplePushConstantData), &push);
    //     
    //     object.model->bind(frameInfo.commandBuffer);
    //     object.model->draw(frameInfo.commandBuffer);
    // }
}

void SimpleVulkanRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    // set按顺序存在vector中，set0,set1,set2 ...
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // 用于传输vertex信息之外的信息（如texture， ubo之类的），目前先没有
    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    // 用于将一些小量的数据送到shader中
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_deviceRef.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void SimpleVulkanRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
    assert(m_pipelineLayout != nullptr && "pipelineLayout is null");
    
    // 使用swapchain的大小而不是Windows的，因为这两个有可能不是一一对应
    PipelineConfigInfo pipelineConfig{};
    VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<VulkanPipeline>(m_deviceRef,
        std::map<EShaderType, std::string>{
        {vs, "../resource/shader/simple_shader.vert.spv"},
        {fs, "../resource/shader/simple_shader.frag.spv"}},
        pipelineConfig);
}

#endif