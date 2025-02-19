#pragma once
#include <unordered_map>

#include "VulkanGraphicsDevice.hpp"

#ifdef RENDER_IN_VULKAN
namespace Kong
{
    class VulkanDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder()=default;

            Builder &AddBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);

            std::unique_ptr<VulkanDescriptorSetLayout> Build();

        private:
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings{};
        };

        VulkanDescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VulkanDescriptorSetLayout();
        
        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
        VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

    private:
        std::shared_ptr<VulkanGraphicsDevice> m_deviceRef;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

        friend class VulkanDescriptorWriter;
    };

    class VulkanDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder() { }

            Builder &AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &SetMaxSets(uint32_t count);
            std::unique_ptr<VulkanDescriptorPool> Build() const;
            
        private:
            std::vector<VkDescriptorPoolSize> m_poolSizes{};
            uint32_t m_maxSets{1000};
            VkDescriptorPoolCreateFlags m_poolFlags {0};
        };

        VulkanDescriptorPool(uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VulkanDescriptorPool();
        
        VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
        VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;

        bool AllocateDescriptor(const VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void FreeDescriptor(std::vector<VkDescriptorSet> &descriptor) const;
        void ResetPool();

    private:
        std::shared_ptr<VulkanGraphicsDevice> m_deviceRef;
        VkDescriptorPool m_descriptorPool;

        friend class VulkanDescriptorWriter;
    };

    class VulkanDescriptorWriter
    {
    public:
        VulkanDescriptorWriter(VulkanDescriptorSetLayout& setLayout, VulkanDescriptorPool& pool);

        VulkanDescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        VulkanDescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool Build(VkDescriptorSet& set);
        void Overwrite(VkDescriptorSet& set);

    private:
        VulkanDescriptorSetLayout& m_setLayout;
        VulkanDescriptorPool& m_pool;
        std::vector<VkWriteDescriptorSet> m_writes;
    };
}
#endif