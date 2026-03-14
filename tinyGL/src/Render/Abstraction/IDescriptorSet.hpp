/**
 * @file IDescriptorSet.hpp
 * @brief RHI 描述符/绑定接口占位，阶段 7。Pipeline 或独立描述符集可绑定 UBO/纹理，由实现层映射到 GL uniform 或 VkDescriptorSet。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    class IBuffer;
    class ITexture;

    /**
     * 与 API 无关的描述符集接口。
     * OpenGL：对应 uniform 槽位绑定；Vulkan：对应 VkDescriptorSet。
     * 默认空实现，后端按需实现。
     */
    class IDescriptorSet
    {
    public:
        virtual ~IDescriptorSet() = default;

        /** 绑定 Uniform Buffer 到指定 slot */
        virtual void BindUniformBuffer(uint32_t slot, IBuffer* buffer) { (void)slot; (void)buffer; }

        /** 绑定纹理到指定 slot */
        virtual void BindTexture(uint32_t slot, ITexture* texture) { (void)slot; (void)texture; }

        IDescriptorSet(const IDescriptorSet&) = delete;
        IDescriptorSet& operator=(const IDescriptorSet&) = delete;

    protected:
        IDescriptorSet() = default;
    };
}
