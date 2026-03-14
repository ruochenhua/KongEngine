/**
 * @file GLBuffer.cpp
 * @brief OpenGL IBuffer 实现。
 * @ingroup RenderAbstraction
 */

#include "GLBuffer.hpp"
#include "glad/glad.h"
#include <stdexcept>
#include <cstring>

namespace Kong
{
    static unsigned int ToGLBufferTarget(BufferUsage usage)
    {
        switch (usage)
        {
        case BufferUsage::Vertex:   return 0x8892; /* GL_ARRAY_BUFFER */
        case BufferUsage::Index:    return 0x8893; /* GL_ELEMENT_ARRAY_BUFFER */
        case BufferUsage::Uniform: return 0x8A11; /* GL_UNIFORM_BUFFER */
        case BufferUsage::Staging:  return 0x8892;
        default: return 0x8892;
        }
    }

    GLBuffer::GLBuffer(const BufferDesc& desc)
        : m_usage(desc.usage)
        , m_isVertex(desc.usage == BufferUsage::Vertex)
    {
        if (desc.size > 0)
            m_size = desc.size;
        else if (desc.instanceCount > 0)
            m_size = desc.instanceCount * 64u; /* fallback for uniform etc. */
        else
            m_size = 256u;

        unsigned int target = ToGLBufferTarget(desc.usage);
        glGenBuffers(1, &m_bufferId);
        glBindBuffer(target, m_bufferId);
        glBufferData(target, static_cast<GLsizeiptr>(m_size), desc.initialData, 0x88E8); /* GL_DYNAMIC_DRAW */

        if (m_isVertex)
            glGenVertexArrays(1, &m_vaoId);

        glBindBuffer(target, 0);

        if (desc.initialData && m_size > 0)
            Upload(desc.initialData, m_size, 0);
    }

    GLBuffer::~GLBuffer()
    {
        if (m_vaoId)
        {
            glDeleteVertexArrays(1, &m_vaoId);
            m_vaoId = 0;
        }
        if (m_bufferId)
        {
            glDeleteBuffers(1, &m_bufferId);
            m_bufferId = 0;
        }
    }

    void GLBuffer::Upload(const void* data, size_t size, size_t offset)
    {
        if (!data) return;
        size_t useSize = (size == 0) ? m_size : size;
        if (useSize == 0 || offset + useSize > m_size) return;

        unsigned int target = ToGLBufferTarget(m_usage);
        glBindBuffer(target, m_bufferId);
        glBufferSubData(target, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(useSize), data);
        glBindBuffer(target, 0);
    }

    void GLBuffer::Bind(uint32_t slot, void* commandList)
    {
        (void)commandList;
        unsigned int target = ToGLBufferTarget(m_usage);
        if (m_usage == BufferUsage::Uniform)
        {
            glBindBufferBase(0x8A11, slot, m_bufferId); /* GL_UNIFORM_BUFFER */
            return;
        }
        if (m_isVertex && m_vaoId)
            glBindVertexArray(m_vaoId);
        else
            glBindBuffer(target, m_bufferId);
    }
}
