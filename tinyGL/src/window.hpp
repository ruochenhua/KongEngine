#pragma once

#include "GLM/vec2.hpp"

struct GLFWwindow;
namespace Kong
{
    class KongWindow
    {
    public:
        static KongWindow& GetWindowModule();
        
        KongWindow();
        GLFWwindow* GetWindow();

        glm::ivec2 windowSize {1280, 960};
        float aspectRatio {1.0f};
        bool resized {false};
    private:

        GLFWwindow* m_window {nullptr};

        static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
    };
    
}
