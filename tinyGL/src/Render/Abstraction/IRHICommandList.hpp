/**
 * @file IRHICommandList.hpp
 * @brief RHI 命令列表：渲染逻辑层通过本接口下发视口、绑定、绘制与屏障，不直接使用 gl/vk。
 * @ingroup RenderAbstraction
 *
 * OpenGL 实现可为即时封装（内部调用 gl*）；Vulkan 实现写入 VkCommandBuffer。
 *
 * 能力审计（阶段 3，渐进迁移时对照）：
 * - SetViewport / SetScissor → glViewport/glScissor 或 vkCmdSetViewport/Scissor
 * - BindFramebuffer / ClearRenderTarget → glBindFramebuffer/glClear 或 render pass + vkCmdClear
 * - BindPipeline → glUseProgram + 固定管线状态 或 vkCmdBindPipeline
 * - BindVertexBuffer / BindIndexBuffer / Draw / DrawIndexed → VAO+VBO/IBO 或 vkCmdBindVertexBuffers + Draw
 * - BindTexture → glActiveTexture+glBindTexture 或 descriptor sets
 * - BindUniformBuffer → glBindBufferBase(GL_UNIFORM_BUFFER) 或 descriptor
 * - TextureBarrier → 多数 GL 为空；Vulkan ImageMemoryBarrier
 */

#pragma once

#include "Render/Abstraction/Types.hpp"

#include <cstddef>
#include <cstdint>

namespace Kong
{
    class IBuffer;
    class ITexture;
    class IFramebuffer;
    class IPipeline;

    /**
     * 单帧内的录制/执行上下文。
     * 约定：由 IFrameContext::GetRHICommandList() 提供；无实现时返回 nullptr，Pass 可走兼容路径。
     */
    class IRHICommandList
    {
    public:
        virtual ~IRHICommandList() = default;

        /** 标记本列表录制开始（Vulkan: command buffer begin；OpenGL 可空） */
        virtual void Begin() = 0;
        /** 标记本列表录制结束 */
        virtual void End() = 0;

        virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.f,
                                 float maxDepth = 1.f) = 0;
        virtual void SetScissor(int x, int y, int width, int height) = 0;

        /**
         * 绑定离屏或交换链目标；nullptr 表示默认 backbuffer（由实现定义）。
         * 多渲染目标应通过 IFramebuffer 封装；OpenGL 实现会为 MRT 设置 glDrawBuffers。
         */
        virtual void BindFramebuffer(IFramebuffer* framebuffer) = 0;

        /**
         * 结束当前渲染通道（Vulkan：vkCmdEndRenderPass；OpenGL：无操作）。
         * 再次 BindFramebuffer 前若仍在 pass 内，Vulkan 实现应先结束当前 pass。
         */
        virtual void EndRenderPass() {}

        /** colorRGBA 可为 nullptr 表示默认清除色（实现定义，常为 0） */
        virtual void ClearRenderTarget(RHIClearMask mask, const float* colorRGBA = nullptr, float depth = 1.f,
                                       uint32_t stencil = 0) = 0;

        virtual void BindPipeline(IPipeline* pipeline) = 0;

        /** 深度写入 / 深度测试（GBuffer 与光照 pass 常用） */
        virtual void SetDepthWriteEnabled(bool enable) { (void)enable; }
        virtual void SetDepthTestEnabled(bool enable) { (void)enable; }

        virtual void BindVertexBuffer(uint32_t slot, IBuffer* buffer, uint64_t offset = 0) = 0;
        virtual void BindIndexBuffer(IBuffer* buffer, IndexElementType indexType, uint64_t offset = 0) = 0;

        virtual void Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount = 1,
                          uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;

        virtual void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount = 1,
                                 uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                                 uint32_t firstInstance = 0) = 0;

        /** 图形绑定槽：采样纹理（实现映射到 texture unit 或 descriptor） */
        virtual void BindTexture(uint32_t slot, ITexture* texture) = 0;

        /** Uniform/Constant 缓冲绑定槽（OpenGL: UBO binding；Vulkan: descriptor 或 push descriptor） */
        virtual void BindUniformBuffer(uint32_t slot, IBuffer* buffer) = 0;

        /**
         * 资源屏障 / 布局迁移（Vulkan 必需；OpenGL 默认可空实现）。
         * oldLayout/newLayout 为实现层约定数值，0 表示「不关心/默认」。
         */
        virtual void TextureBarrier(ITexture* texture, uint32_t oldLayout, uint32_t newLayout) { (void)texture; (void)oldLayout; (void)newLayout; }

        IRHICommandList(const IRHICommandList&) = delete;
        IRHICommandList& operator=(const IRHICommandList&) = delete;

    protected:
        IRHICommandList() = default;
    };
}
