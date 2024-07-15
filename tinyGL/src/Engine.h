#pragma once
#include "common.h"

namespace tinyGL
{
    class Engine
    {
    public:
        Engine();
        GLFWwindow* GetRenderWindow();
        
        static Engine GetEngine();
    private:
        
        GLFWwindow* render_window = nullptr;
    };

}
