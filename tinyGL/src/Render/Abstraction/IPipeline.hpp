/**
 * @file IPipeline.hpp
 * @brief RHI 管线接口占位，阶段 5 实现。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    class IFrameContext;
    class IRHICommandList;

    /** 与 API 无关的图形管线；IRHICommandList::BindPipeline 优先调用 BindGraphics。 */
    class IPipeline
    {
    public:
        virtual ~IPipeline() = default;

        /** 兼容旧路径（仅 IFrameContext、无 CommandList 时） */
        virtual void Bind(IFrameContext& frameContext) = 0;

        /**
         * 绑定到当前录制中的命令列表（推荐）。
         * 默认空实现；OpenGL/Vulkan 后端应覆盖以执行 glUseProgram / vkCmdBindPipeline。
         */
        virtual void BindGraphics(IRHICommandList* cmd) { (void)cmd; }

        IPipeline(const IPipeline&) = delete;
        IPipeline& operator=(const IPipeline&) = delete;

    protected:
        IPipeline() = default;
    };
}
