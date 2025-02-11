#pragma once
#include "common.h"

namespace Kong
{
    enum GraphicsAPI
    {
        VULKAN,
        OPENGL,
        NONE
    };
    
    class GraphicsDevice
    {
    public:
        
        virtual ~GraphicsDevice() = default;
        virtual GLFWwindow* Init(int width, int height) = 0;
        // virtual void BeginFrame() = 0;
        // virtual void EndFrame() = 0;
        // virtual void Destroy() = 0;

    protected:
        GraphicsAPI m_API {NONE};
    };
}
