#pragma once

#include <imgui.h>
// #include <imgui_impl_vulkan.h>

#include "Window.hpp"

namespace Kong
{
    constexpr int TIME_RECORD_COUNT = 100;
    // ui相关内容
    class KongUIManager
    {
    public:
        static KongUIManager& GetUIManager();
        KongUIManager();
        
        void Init(GLFWwindow* windowHandle);
        void PreRenderUpdate(double delta);
        void PostRenderUpdate();
        void Destroy();

    private:
        void DescribeUIContent(double delta);
        ImVec4 GetFrameRateColor(int framerate);

        float process_time[TIME_RECORD_COUNT];
        int process_time_offset {0};
#ifdef RENDER_IN_VULKAN
        // ImGui_ImplVulkanH_Window m_mainWindowData;
#endif
        
    };
}
