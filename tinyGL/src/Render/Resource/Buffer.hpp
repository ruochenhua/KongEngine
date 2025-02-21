#pragma once
#include <stdint.h>
namespace Kong
{
    enum BufferType : short
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
        virtual ~KongBuffer() = default;
        
        virtual void Initialize(BufferType type, uint64_t size, uint32_t instanceCount, void* data = nullptr)
        {
            m_type = type;
            m_isValid = true;
        }
        
        KongBuffer(const KongBuffer& other) = delete;
        KongBuffer& operator=(const KongBuffer& other) = delete;

        virtual void Bind(void* commandBuffer = nullptr)
        {
            
        }

        bool IsValid() const {return m_isValid;}
    protected:
        BufferType m_type {NONE_BUFFER};
        bool m_isValid {false};
    };
}
