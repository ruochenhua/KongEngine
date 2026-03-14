/**
 * @file IBuffer.hpp
 * @brief RHI 缓冲接口，双后端统一抽象。
 * @ingroup RenderAbstraction
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace Kong
{
    /**
     * 与 API 无关的缓冲接口。
     * - OpenGL: 对应 VBO/IBO/UBO，Bind(slot) 即 glBindBufferBase 等。
     * - Vulkan: 对应 VkBuffer，Bind 在 descriptor 侧体现；Map/Flush 可在实现内部使用。
     * 线程安全：由实现决定；通常每帧主线程使用。
     */
    class IBuffer
    {
    public:
        virtual ~IBuffer() = default;

        /** 上传数据到缓冲，offset 字节偏移，size 字节长度，若 size==0 表示全量 */
        virtual void Upload(const void* data, size_t size = 0, size_t offset = 0) = 0;

        /** 绑定到指定槽位（如 Uniform 的 binding、顶点属性等），commandList 为 GetCurrentCommandList() 或 nullptr（GL） */
        virtual void Bind(uint32_t slot, void* commandList = nullptr) = 0;

        IBuffer(const IBuffer&) = delete;
        IBuffer& operator=(const IBuffer&) = delete;

    protected:
        IBuffer() = default;
    };
}
