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
        float GetAspectRatio() const {return window_aspect_ratio;}
        void  SetWidthHeight(float width, float height);
    private:
        float window_width = 1024.f;
        float window_height = 768.f;
        float window_aspect_ratio = 1.f;
        GLFWwindow* render_window = nullptr;
    };

}
