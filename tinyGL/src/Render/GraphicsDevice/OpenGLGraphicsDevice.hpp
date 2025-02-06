#pragma once
#include "GraphicsDevice.hpp"

namespace Kong
{
    class OpenGLGraphicsDevice : public GraphicsDevice
    {
        public:
        OpenGLGraphicsDevice();
        virtual ~OpenGLGraphicsDevice();

        void Init() override;
        void BeginFrame() override;
        void EndFrame() override;
        void Destroy() override;
    };
}
