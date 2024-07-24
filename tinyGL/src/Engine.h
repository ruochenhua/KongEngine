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

        static string ReadFile(const string& file_path);
    private:
        
        GLFWwindow* render_window = nullptr;
    };

}
