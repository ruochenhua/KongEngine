/**
 * @file GLFrameContext.hpp
 * @brief OpenGL 实现的 IFrameContext，轻量封装无显式命令列表。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/IFrameContext.hpp"

namespace Kong
{
    /** OpenGL 后端 IFrameContext：无显式帧边界，GetCurrentCommandList 返回 nullptr */
    class GLFrameContext : public IFrameContext
    {
    public:
        void BeginFrame() override {}
        void EndFrame() override {}
        void* GetCurrentCommandList() override { return nullptr; }
        int GetFrameIndex() const override { return 0; }
    };
}
