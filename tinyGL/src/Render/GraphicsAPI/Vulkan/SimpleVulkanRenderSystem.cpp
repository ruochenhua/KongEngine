#include "SimpleVulkanRenderSystem.hpp"

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
};

SimpleVulkanRenderSystem::SimpleVulkanRenderSystem(VkRenderPass renderPass)
    : m_deviceRef(VulkanGraphicsDevice::GetGraphicsDevice())
{
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline(renderPass);
}

SimpleVulkanRenderSystem::~SimpleVulkanRenderSystem()
{
    vkDestroyPipelineLayout(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), m_pipelineLayout, nullptr);
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

        mesh_component->CreateMeshDescriptorSet(m_descriptorSetLayout, m_descriptorPool.get());
    }
}

void SimpleVulkanRenderSystem::RenderGameObjects(const FrameInfo& frameInfo)
{
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

        vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(SimplePushConstantData), &push);
       
        mesh_component->Draw(frameInfo, m_pipelineLayout);
    }
}

void SimpleVulkanRenderSystem::CreateDescriptorSetLayout()
{
    int meshCount = 10 * VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
    int meshTexCount = 5;
    m_descriptorPool = VulkanDescriptorPool::Builder()
               .SetMaxSets(meshCount)  // 简单设置一个最大数量
               .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshCount)
               .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, meshCount*meshTexCount)
               .Build();
    
    auto basicLayout = VulkanDescriptorSetLayout::Builder()
    // projection view, 有无贴图等数据
    
    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 1)
    .Build();
    basicLayout->m_usage = VulkanDescriptorSetLayout::BasicData;
    m_descriptorSetLayout.push_back(std::move(basicLayout));
    
    auto textureLayout = VulkanDescriptorSetLayout::Builder()
    // image 贴图数据
    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // albedo
    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
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
    for (auto& layout : m_descriptorSetLayout)
    {
        descriptorSetLayouts.push_back(layout->GetDescriptorSetLayout());
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // 用于传输vertex信息之外的信息（如texture， ubo之类的），目前先没有
    // descriptor set layout
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    // 用于将一些小量的数据送到shader中
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_deviceRef->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
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
    m_pipeline = std::make_unique<VulkanPipeline>(std::map<EShaderType, std::string>{
        {vs, "shader/Vulkan/simple_shader.vulkan.vert.spv"},
        {fs, "shader/Vulkan/simple_shader.vulkan.frag.spv"}},
        pipelineConfig);
}

#endif