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
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
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

void OpenGLBuffer::AddAttribute(const std::vector<OpenGLVertexAttribute>& attributes) const
{
    int size = attributes.size();
    for (int i = 0; i < size; i++)
    {
        auto& attr = attributes[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, attr.size, attr.type, attr.normalized,  attr.stride, attr.offset);
    }
}

void OpenGLBuffer::Bind(void* commandBuffer)
{
    glBindVertexArray(m_VAO);
}