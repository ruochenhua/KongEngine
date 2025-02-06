#pragma once

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
        virtual void Init() = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Destroy() = 0;

    protected:
        GraphicsAPI m_API {NONE};
    };
}