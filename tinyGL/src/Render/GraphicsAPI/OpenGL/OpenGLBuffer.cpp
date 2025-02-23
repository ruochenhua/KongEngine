#include "OpenGLBuffer.hpp"

#include <memory>
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

OpenGLRenderInfo::OpenGLRenderInfo()
{
    material = std::make_shared<RenderMaterialInfo>();
}

void OpenGLRenderInfo::Draw(void* commandBuffer)
{
    vertex_buffer->Bind(commandBuffer);

    // Draw the triangle !
    // if no index, use draw array
    if(!index_buffer->IsValid())
    {
        if(instance_buffer != GL_NONE)
        {
            // Starting from vertex 0; 3 vertices total -> 1 triangle
            glDrawArraysInstanced(GL_TRIANGLES, 0,vertices.size(),instance_count);
        }
        else
        {
            // Starting from vertex 0; 3 vertices total -> 1 triangle
            glDrawArrays(GL_TRIANGLES, 0, vertices.size()); 	
        }
    }
    else
    {
        if(instance_buffer != GL_NONE)
        {
            glDrawElementsInstanced(GL_TRIANGLES, m_Index.size(), GL_UNSIGNED_INT, 0, instance_count);
        }
        else
        {
            glDrawElements(GL_TRIANGLES, m_Index.size(), GL_UNSIGNED_INT, 0);
        }
    }
}

void OpenGLRenderInfo::InitRenderInfo()
{
    // //init vertex buffer
    vertex_buffer = std::make_unique<OpenGLBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), vertices.size(), &vertices[0]);
    std::vector<OpenGLVertexAttribute> vertexAttributes = {
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)},
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)},
        {2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)},
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent)},
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent)},
    };
    
    dynamic_cast<OpenGLBuffer*>(vertex_buffer.get())->AddAttribute(vertexAttributes);

    // index buffer
    if(!m_Index.empty())
    {
        index_buffer = std::make_unique<OpenGLBuffer>();
        index_buffer->Initialize(INDEX_BUFFER, sizeof(unsigned int), m_Index.size(), &m_Index[0]);
    }
}
