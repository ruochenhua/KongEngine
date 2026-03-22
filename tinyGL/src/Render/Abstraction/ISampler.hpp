/**
 * @file ISampler.hpp
 * @brief RHI 采样器对象（过滤、环绕、各向异性等），与 ITexture 分离以便 Vulkan 对齐。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    /**
     * 与 API 无关的采样器。
     * OpenGL：可实现为无独立对象（绑定纹理时推状态），或 glSamplerParameteri。
     * Vulkan：VkSampler。
     */
    class ISampler
    {
    public:
        virtual ~ISampler() = default;

        /** 绑定到纹理槽位，与 ITexture::Bind 配合或覆盖该槽采样状态，由实现定义 */
        virtual void Bind(uint32_t slot, void* commandList = nullptr) = 0;

        ISampler(const ISampler&) = delete;
        ISampler& operator=(const ISampler&) = delete;

    protected:
        ISampler() = default;
    };
}
