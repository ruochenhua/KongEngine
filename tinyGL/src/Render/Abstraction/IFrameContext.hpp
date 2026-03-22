/**
 * @file IFrameContext.hpp
 * @brief RHI 帧上下文接口，表示当前帧的提交上下文。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    class IRHICommandList;

    /**
     * 与 API 无关的帧上下文。
     * - OpenGL: 轻量封装，BeginFrame/EndFrame 可空或仅状态重置；GetCurrentCommandList() 返回 nullptr。
     * - Vulkan: 对应当前帧的 VkCommandBuffer，BeginFrame 内 acquire image、begin command buffer，EndFrame 内 submit、present。
     * 使用约定：单帧内 BeginFrame 后、EndFrame 前，主线程顺序调用各 IRenderSystem::Draw。
     */
    class IFrameContext
    {
    public:
        virtual ~IFrameContext() = default;

        /** 开始本帧录制/状态准备 */
        virtual void BeginFrame() = 0;

        /** 结束本帧并提交（Vulkan submit + present），OpenGL 可能仅 SwapBuffers 在窗口层 */
        virtual void EndFrame() = 0;

        /**
         * 获取当前帧命令列表，供实现层内部使用。
         * OpenGL 返回 nullptr；Vulkan 返回当前 VkCommandBuffer。
         */
        virtual void* GetCurrentCommandList() = 0;

        /** 当前帧索引（多缓冲用）。OpenGL 恒为 0，Vulkan 为 swapchain 帧索引 */
        virtual int GetFrameIndex() const = 0;

        /**
         * 本帧 RHI 命令列表；OpenGL 为 OpenGLCommandList，Vulkan 为 VulkanRHICommandList。
         */
        virtual IRHICommandList* GetRHICommandList() { return nullptr; }

        IFrameContext(const IFrameContext&) = delete;
        IFrameContext& operator=(const IFrameContext&) = delete;

    protected:
        IFrameContext() = default;
    };
}
