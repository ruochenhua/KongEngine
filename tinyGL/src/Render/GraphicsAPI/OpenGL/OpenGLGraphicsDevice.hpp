#pragma once
#include "../GraphicsDevice.hpp"

namespace Kong
{
    class OpenGLGraphicsDevice : public GraphicsDevice
    {
    public:
        static OpenGLGraphicsDevice* GetGraphicsDevice();
            
        OpenGLGraphicsDevice();
        ~OpenGLGraphicsDevice() override;

        GLFWwindow* Init(int width, int height) override;
    };
}
