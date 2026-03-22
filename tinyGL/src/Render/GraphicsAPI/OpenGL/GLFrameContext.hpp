#pragma once

#include "Render/Abstraction/IFrameContext.hpp"
#include "OpenGLCommandList.hpp"

namespace Kong
{
    class GLFrameContext : public IFrameContext
    {
    public:
        void BeginFrame() override {}
        void EndFrame() override {}
        void* GetCurrentCommandList() override { return nullptr; }
        int GetFrameIndex() const override { return 0; }
        IRHICommandList* GetRHICommandList() override { return &m_cmdList; }

    private:
        OpenGLCommandList m_cmdList;
    };
}
