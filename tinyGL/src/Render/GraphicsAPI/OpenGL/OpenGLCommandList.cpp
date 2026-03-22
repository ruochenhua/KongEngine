#include "OpenGLCommandList.hpp"
#include "GLBuffer.hpp"
#include "GLFramebuffer.hpp"
#include "Render/Abstraction/IPipeline.hpp"
#include "Render/Abstraction/ITexture.hpp"

#include "glad/glad.h"

namespace Kong
{
    void OpenGLCommandList::Begin() {}

    void OpenGLCommandList::End() {}

    void OpenGLCommandList::SetViewport(float x, float y, float width, float height, float minDepth,
                                        float maxDepth)
    {
        glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width),
                   static_cast<GLsizei>(height));
        glDepthRangef(minDepth, maxDepth);
    }

    void OpenGLCommandList::SetScissor(int x, int y, int width, int height)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
    }

    void OpenGLCommandList::BindFramebuffer(IFramebuffer* framebuffer)
    {
        if (!framebuffer)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDrawBuffer(GL_BACK);
            return;
        }
        auto* glfb = dynamic_cast<GLFramebuffer*>(framebuffer);
        if (!glfb)
            return;
        glBindFramebuffer(GL_FRAMEBUFFER, glfb->GetGLName());
        glfb->ApplyDrawBuffers();
    }

    void OpenGLCommandList::EndRenderPass() {}

    void OpenGLCommandList::ClearRenderTarget(RHIClearMask mask, const float* colorRGBA, float depth,
                                              uint32_t stencil)
    {
        GLbitfield glMask = 0;
        if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(RHIClearMask::Color)) != 0)
        {
            glMask |= GL_COLOR_BUFFER_BIT;
            if (colorRGBA)
                glClearColor(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]);
            else
                glClearColor(0.f, 0.f, 0.f, 1.f);
        }
        if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(RHIClearMask::Depth)) != 0)
        {
            glMask |= GL_DEPTH_BUFFER_BIT;
            glClearDepthf(depth);
        }
        if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(RHIClearMask::Stencil)) != 0)
        {
            glMask |= GL_STENCIL_BUFFER_BIT;
            glClearStencil(static_cast<GLint>(stencil));
        }
        if (glMask != 0)
            glClear(glMask);
    }

    void OpenGLCommandList::BindPipeline(IPipeline* pipeline)
    {
        if (pipeline)
            pipeline->BindGraphics(this);
    }

    void OpenGLCommandList::SetDepthWriteEnabled(bool enable)
    {
        glDepthMask(enable ? GL_TRUE : GL_FALSE);
    }

    void OpenGLCommandList::SetDepthTestEnabled(bool enable)
    {
        if (enable)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void OpenGLCommandList::BindVertexBuffer(uint32_t slot, IBuffer* buffer, uint64_t offset)
    {
        (void)slot;
        (void)offset;
        auto* glb = dynamic_cast<GLBuffer*>(buffer);
        if (!glb)
            return;
        glb->Bind(0, nullptr);
    }

    void OpenGLCommandList::BindIndexBuffer(IBuffer* buffer, IndexElementType indexType, uint64_t offset)
    {
        (void)offset;
        auto* glb = dynamic_cast<GLBuffer*>(buffer);
        if (!glb)
        {
            m_hasIndexBuffer = false;
            return;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glb->GetGLBufferId());
        m_boundIndexType = indexType;
        m_hasIndexBuffer = true;
    }

    void OpenGLCommandList::Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount,
                                 uint32_t firstVertex, uint32_t firstInstance)
    {
        (void)firstInstance;
        GLenum mode = TopologyToGL(topology);
        if (instanceCount <= 1)
            glDrawArrays(mode, static_cast<GLint>(firstVertex), static_cast<GLsizei>(vertexCount));
        else
            glDrawArraysInstanced(mode, static_cast<GLint>(firstVertex), static_cast<GLsizei>(vertexCount),
                                  static_cast<GLsizei>(instanceCount));
    }

    void OpenGLCommandList::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount,
                                        uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                        uint32_t firstInstance)
    {
        (void)firstInstance;
        if (!m_hasIndexBuffer)
            return;
        GLenum mode = TopologyToGL(topology);
        GLenum idxType = IndexTypeToGL(m_boundIndexType);
        const unsigned elemBytes = m_boundIndexType == IndexElementType::UInt16 ? 2u : 4u;
        const void* indices = reinterpret_cast<const void*>(static_cast<uintptr_t>(firstIndex * elemBytes));
        if (instanceCount <= 1)
            glDrawElements(mode, static_cast<GLsizei>(indexCount), idxType, indices);
        else
            glDrawElementsInstanced(mode, static_cast<GLsizei>(indexCount), idxType, indices,
                                    static_cast<GLsizei>(instanceCount));
    }

    void OpenGLCommandList::BindTexture(uint32_t slot, ITexture* texture)
    {
        if (texture)
            texture->Bind(slot, nullptr);
    }

    void OpenGLCommandList::BindUniformBuffer(uint32_t slot, IBuffer* buffer)
    {
        if (buffer)
            buffer->Bind(slot, nullptr);
    }

    GLenum OpenGLCommandList::IndexTypeToGL(IndexElementType t) const
    {
        switch (t)
        {
        case IndexElementType::UInt16: return GL_UNSIGNED_SHORT;
        case IndexElementType::UInt32:
        default: return GL_UNSIGNED_INT;
        }
    }

    GLenum OpenGLCommandList::TopologyToGL(PrimitiveTopology t) const
    {
        switch (t)
        {
        case PrimitiveTopology::PointList: return GL_POINTS;
        case PrimitiveTopology::LineList: return GL_LINES;
        case PrimitiveTopology::LineStrip: return GL_LINE_STRIP;
        case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleList:
        default: return GL_TRIANGLES;
        }
    }
}
