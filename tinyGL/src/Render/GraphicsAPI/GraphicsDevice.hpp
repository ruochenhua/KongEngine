#pragma once
#include "common.h"
#include "Render/Abstraction/IGraphicsDevice.hpp"

namespace Kong
{
    enum GraphicsAPI
    {
        VULKAN,
        OPENGL,
        NONE
    };

    /** 图形设备基类，实现 RHI 层 IGraphicsDevice 接口 */
    class GraphicsDevice : public IGraphicsDevice
    {
    public:
        ~GraphicsDevice() override = default;

        /** 初始化窗口与上下文；返回窗口句柄（如 GLFWwindow*） */
        void* Init(int width, int height) override = 0;

    protected:
        GraphicsAPI m_API {NONE};
    };
}
