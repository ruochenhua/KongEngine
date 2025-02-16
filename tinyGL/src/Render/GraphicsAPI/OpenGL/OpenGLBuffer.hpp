#pragma once
#include <cstdint>

#include "glad/glad.h"
#include "Render/Resource/Buffer.hpp"

namespace Kong
{
    class OpenGLBuffer : public KongBuffer
    {
    public:
        OpenGLBuffer() = default;
        void Initialize(BufferType type,
            uint64_t instanceSize,
            uint32_t instanceCount, void* data = nullptr) override;

        GLuint GetBuffer() const {return m_ID;}
    private:
        GLuint m_ID {GL_NONE};    
    };
}
