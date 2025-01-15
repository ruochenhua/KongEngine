#pragma once
#include <imgui.h>

namespace Kong
{
    constexpr int TIME_RECORD_COUNT = 100;
    // ui相关内容
    class CUIManager
    {
    public:
        static CUIManager* GetUIManager();
        
        void Init();
        void PreRenderUpdate(double delta);
        void PostRenderUpdate();
        void Destroy();

    private:
        void DescribeUIContent(double delta);
        ImVec4 GetFrameRateColor(int framerate);

        float process_time[TIME_RECORD_COUNT];
        int process_time_offset = 0;
    };
}
