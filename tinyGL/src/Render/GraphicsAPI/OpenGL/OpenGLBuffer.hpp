#pragma once
#include <cstdint>
#include <vector>

#include "glad/glad.h"
#include "Render/Resource/Buffer.hpp"

namespace Kong
{
    struct OpenGLVertexAttribute
    {
        GLint size{0};
        GLenum type{0};
        GLboolean normalized{0};
        GLsizei stride{0};
        void* offset;
    };
    
    class OpenGLBuffer : public KongBuffer
    {
    public:
        OpenGLBuffer() = default;
        void Initialize(BufferType type,
            uint64_t instanceSize,
            uint32_t instanceCount, void* data = nullptr) override;

        void AddAttribute(const std::vector<OpenGLVertexAttribute>& attributes) const;
        
        GLuint GetBuffer() const {return m_ID;}
        void Bind(void* commandBuffer) override;
    private:
        GLuint m_ID {GL_NONE};
        GLuint m_VAO {GL_NONE}; // vertex buffer会创建VAO
    };
}
