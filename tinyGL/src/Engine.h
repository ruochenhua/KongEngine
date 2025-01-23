#pragma once
#include "Common.h"

namespace Kong
{
    class Engine
    {
    public:
        Engine();
        // GLFWwindow* GetRenderWindow();
        
        static Engine GetEngine();
        // static GLFWwindow* GetRenderWindow();

        static string ReadFile(const string& file_path);
        // float GetAspectRatio() const {return window_aspect_ratio;}
        
        // void  SetWidthHeight(int width, int height);
        // glm::ivec2 GetWindowSize() const {return window_size;}
    private:
        // glm::ivec2 window_size = glm::ivec2(1024, 768);
        // float window_aspect_ratio = 1.f;
        // GLFWwindow* render_window = nullptr;
    };

}
