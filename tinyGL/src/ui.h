#pragma once
#include <imgui.h>

namespace tinyGL
{
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
    };
}
