#include "OpenGLBuffer.hpp"

#include <stdexcept>

using namespace Kong;

void OpenGLBuffer::Initialize(BufferType type,
    uint64_t instanceSize,
    uint32_t instanceCount, void* data)
{
    KongBuffer::Initialize(type, instanceSize, instanceCount);
    glGenBuffers(1, &m_ID);
    GLenum bufferType = GL_NONE;
    switch (type)
    {
    case VERTEX_BUFFER:
        bufferType = GL_ARRAY_BUFFER;
        break;
    case INDEX_BUFFER:
        bufferType = GL_ELEMENT_ARRAY_BUFFER;
        break;
    case UNIFORM_BUFFER:
        bufferType = GL_UNIFORM_BUFFER;
        break;
    default:
        throw std::runtime_error("Invalid Buffer Type");
        break;
    }

    glBindBuffer(bufferType, m_ID);
    glBufferData(bufferType, instanceSize*instanceCount, data, GL_DYNAMIC_DRAW);
}
