#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanGraphicsDevice.hpp"
#include "Render/RenderCommon.hpp"

namespace Kong
{
    struct Vertex
    {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};
            
        static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
    };
    
    struct PipelineConfigInfo
    {
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        // 设定input assembly阶段的信息（顶点信息组装成三角形阶段）
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        VkPipelineLayout pipelineLayout {nullptr};
        VkRenderPass renderPass {nullptr};
        uint32_t subpass {0};
    };
    
    class VulkanPipeline
    {
    public:
        VulkanPipeline(VulkanGraphicsDevice& device,
            std::map<EShaderType, std::string>& shaderPaths,
            const PipelineConfigInfo& configInfo);
        ~VulkanPipeline();

        VulkanPipeline(const VulkanPipeline&) = delete;
        VulkanPipeline& operator=(const VulkanPipeline&) = delete;

        void Bind(const VkCommandBuffer& commandBuffer);
        static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        
    private:
        void CreateGraphicsPipeline(std::map<EShaderType, std::string>& shaderPaths, const PipelineConfigInfo& configInfo);
        void CreateShaderModule(const std::string& shaderCode, VkShaderModule* shaderModule);
        
        VulkanGraphicsDevice& m_deviceRef;
        VkPipeline m_graphicsPipeline;
        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;
    };
}
