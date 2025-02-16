#pragma once

namespace Kong
{
    enum BufferType : uint8_t
    {
        VERTEX_BUFFER = 0,
        INDEX_BUFFER,
        UNIFORM_BUFFER,
        NONE_BUFFER,
        
    };
    class KongBuffer
    {
    public:
        KongBuffer() = default;

        virtual void Initialize(BufferType type, uint64_t size, uint32_t instanceCount, void* data = nullptr)
        {
            m_type = type;
        }
        
        KongBuffer(const KongBuffer& other) = delete;
        KongBuffer& operator=(const KongBuffer& other) = delete;

    protected:
        BufferType m_type {NONE_BUFFER};
    };
}
