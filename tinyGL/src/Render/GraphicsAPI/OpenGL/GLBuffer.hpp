/**
 * @file GLBuffer.hpp
 * @brief OpenGL 实现的 IBuffer，RHI 缓冲接口。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/IBuffer.hpp"
#include "Render/Abstraction/Types.hpp"
#include <cstddef>
#include <cstdint>

struct GLFWwindow;

namespace Kong
{
    /** OpenGL 后端 IBuffer 实现 */
    class GLBuffer : public IBuffer
    {
    public:
        explicit GLBuffer(const BufferDesc& desc);
        ~GLBuffer() override;

        void Upload(const void* data, size_t size = 0, size_t offset = 0) override;
        void Bind(uint32_t slot, void* commandList = nullptr) override;

    private:
        unsigned int m_bufferId {0};
        unsigned int m_vaoId {0};       ///< 仅顶点缓冲使用
        uint64_t     m_size {0};
        BufferUsage  m_usage {BufferUsage::Vertex};
        bool         m_isVertex {false};
    };
}
