#pragma once

#include <imgui.h>
#include "Window.hpp"

namespace Kong
{
    class IGraphicsDevice;

    constexpr int TIME_RECORD_COUNT = 100;
    // ui相关内容
    class KongUIManager
    {
    public:
        static KongUIManager& GetUIManager();
        KongUIManager();

        /** 初始化 ImGui；device 可为空，非空时按 GetBackendType() 选择 Vulkan 或 OpenGL 后端 */
        void Init(GLFWwindow* windowHandle, IGraphicsDevice* device = nullptr);
        void PreRenderUpdate(double delta);
        void PostRenderUpdate();
        void Destroy();

    private:
        void DescribeUIContent(double delta);
        ImVec4 GetFrameRateColor(int framerate);

        float process_time[TIME_RECORD_COUNT];
        int process_time_offset {0};
        bool m_imguiVulkan {false};  ///< true 表示 ImGui 使用 Vulkan 后端
        void* m_imguiDescriptorPool {nullptr};  ///< Vulkan 时存 VkDescriptorPool，Destroy 时释放
    };
}
