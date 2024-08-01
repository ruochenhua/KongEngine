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
        
        void  SetWidthHeight(int width, int height);
        int GetWindowWidth() const {return window_width;}
        int GetWindowHeight() const {return window_height;}
    private:
        int window_width = 1024;
        int window_height = 768;
        float window_aspect_ratio = 1.f;
        GLFWwindow* render_window = nullptr;
    };

}
