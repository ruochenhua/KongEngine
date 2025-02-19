#include "VulkanDescriptor.hpp"
#ifdef RENDER_IN_VULKAN

using namespace Kong;


// *************** Descriptor Set Layout Builder *********************
VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::AddBinding(uint32_t binding,
    VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count)
{
    // 重复的binding
    assert(m_bindings.count(binding) == 0 && "Binding already exists");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.descriptorCount = count;
    m_bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayout::Builder::Build()
{
    return std::make_unique<VulkanDescriptorSetLayout>( m_bindings);
}

// *************** Descriptor Set Layout *********************
VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : m_deviceRef(VulkanGraphicsDevice::GetGraphicsDevice()), m_bindings(bindings)
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto binding : m_bindings)
    {
        setLayoutBindings.push_back(binding.second);
    }

    VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
    setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    setLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_deviceRef->GetDevice(), &setLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_deviceRef->GetDevice(), m_descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************
VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType,
    uint32_t count)
{
    m_poolSizes.push_back({descriptorType, count});
    return *this;
}

VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    m_poolFlags = flags;
    return *this;
}

VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::SetMaxSets(uint32_t count)
{
    m_maxSets = count;
    return *this;
}

std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPool::Builder::Build() const
{
    return make_unique<VulkanDescriptorPool>(m_maxSets, m_poolFlags, m_poolSizes);
}

// *************** Descriptor Pool *********************
VulkanDescriptorPool::VulkanDescriptorPool(uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
        : m_deviceRef(VulkanGraphicsDevice::GetGraphicsDevice())
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(m_deviceRef->GetDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
    vkDestroyDescriptorPool(m_deviceRef->GetDevice(), m_descriptorPool, nullptr);
}

bool VulkanDescriptorPool::AllocateDescriptor(const VkDescriptorSetLayout& descriptorSetLayout,
    VkDescriptorSet& descriptor) const
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(m_deviceRef->GetDevice(), &allocInfo, &descriptor) != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

void VulkanDescriptorPool::FreeDescriptor(std::vector<VkDescriptorSet>& descriptor) const
{
    vkFreeDescriptorSets(
        m_deviceRef->GetDevice(), m_descriptorPool,
        static_cast<uint32_t>(descriptor.size()), descriptor.data());
}

void VulkanDescriptorPool::ResetPool()
{
    vkResetDescriptorPool(m_deviceRef->GetDevice(), m_descriptorPool, 0);
}

// *************** Descriptor Writer *********************
VulkanDescriptorWriter::VulkanDescriptorWriter(VulkanDescriptorSetLayout& setLayout, VulkanDescriptorPool& pool)
    : m_setLayout(setLayout), m_pool(pool)
{
}

VulkanDescriptorWriter& VulkanDescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
    assert(m_setLayout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");
 
    auto &bindingDescription = m_setLayout.m_bindings[binding];
 
    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");
 
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;
 
    m_writes.push_back(write);
    return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
    assert(m_setLayout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");
 
    auto &bindingDescription = m_setLayout.m_bindings[binding];
 
    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");
 
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;
 
    m_writes.push_back(write);
    return *this;
}

bool VulkanDescriptorWriter::Build(VkDescriptorSet& set)
{
    bool success = m_pool.AllocateDescriptor(m_setLayout.GetDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    Overwrite(set);
    return true;
}

void VulkanDescriptorWriter::Overwrite(VkDescriptorSet& set)
{
    for (auto &write : m_writes)
    {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(m_pool.m_deviceRef->GetDevice(),
        m_writes.size(),
        m_writes.data(),
        0, nullptr);
}


#endif
