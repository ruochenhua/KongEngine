/**
 * @file IPipeline.hpp
 * @brief RHI 管线接口占位，阶段 5 实现。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    class IFrameContext;

    /** 与 API 无关的管线接口；Bind 后后续绘制使用该管线状态 */
    class IPipeline
    {
    public:
        virtual ~IPipeline() = default;
        virtual void Bind(IFrameContext& frameContext) = 0;
        IPipeline(const IPipeline&) = delete;
        IPipeline& operator=(const IPipeline&) = delete;
    protected:
        IPipeline() = default;
    };
}
