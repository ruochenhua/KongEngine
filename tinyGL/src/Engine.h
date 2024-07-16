#pragma once
#include "common.h"

namespace tinyGL
{
    class Engine
    {
    public:
        Engine();
        // GLFWwindow* GetRenderWindow();
        
        static Engine GetEngine();
        static GLFWwindow* GetRenderWindow();
    private:
        
        GLFWwindow* render_window = nullptr;
    };

}
