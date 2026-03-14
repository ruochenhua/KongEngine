/**
 * @file IRenderPass.hpp
 * @brief RHI 渲染通道接口占位，阶段 5 实现。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    class IFrameContext;
    class IFramebuffer;

    /** 与 API 无关的渲染通道；描述一次 Pass 的附件与 load/store */
    class IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;
        virtual void Begin(IFrameContext& frameContext, IFramebuffer* framebuffer) = 0;
        virtual void End(IFrameContext& frameContext) = 0;
        IRenderPass(const IRenderPass&) = delete;
        IRenderPass& operator=(const IRenderPass&) = delete;
    protected:
        IRenderPass() = default;
    };
}
