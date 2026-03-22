#pragma once

#include "Render/Abstraction/IRHICommandList.hpp"
#include "glad/glad.h"

namespace Kong
{
    class OpenGLCommandList : public IRHICommandList
    {
    public:
        void Begin() override;
        void End() override;

        void SetViewport(float x, float y, float width, float height, float minDepth = 0.f,
                         float maxDepth = 1.f) override;
        void SetScissor(int x, int y, int width, int height) override;

        void BindFramebuffer(IFramebuffer* framebuffer) override;

        void ClearRenderTarget(RHIClearMask mask, const float* colorRGBA = nullptr, float depth = 1.f,
                               uint32_t stencil = 0) override;

        void BindPipeline(IPipeline* pipeline) override;

        void BindVertexBuffer(uint32_t slot, IBuffer* buffer, uint64_t offset = 0) override;
        void BindIndexBuffer(IBuffer* buffer, IndexElementType indexType, uint64_t offset = 0) override;

        void Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount = 1,
                  uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;

        void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount = 1,
                         uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                         uint32_t firstInstance = 0) override;

        void BindTexture(uint32_t slot, ITexture* texture) override;
        void BindUniformBuffer(uint32_t slot, IBuffer* buffer) override;

    private:
        GLenum IndexTypeToGL(IndexElementType t) const;
        GLenum TopologyToGL(PrimitiveTopology t) const;

        IndexElementType m_boundIndexType {IndexElementType::UInt32};
        bool             m_hasIndexBuffer {false};
    };
}
