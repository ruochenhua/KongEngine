#pragma once

#include "GLM/vec2.hpp"
struct GLFWwindow;
namespace Kong
{
    class IGraphicsDevice;

    class KongWindow
    {
    public:
        static KongWindow& GetWindowModule();

        KongWindow();
        ~KongWindow();
        GLFWwindow* GetWindow();
        /** 获取当前 RHI 设备，用于 BeginFrame/EndFrame 等 */
        IGraphicsDevice* GetGraphicsDevice() { return m_device; }

        glm::ivec2 windowSize {1280, 960};
        float aspectRatio {1.0f};
        bool resized {false};
    private:
        GLFWwindow* m_window {nullptr};
        IGraphicsDevice* m_device {nullptr};

        static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
    };
}
