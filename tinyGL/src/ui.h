#pragma once

namespace tinyGL
{
    // ui相关内容
    class CUIManager
    {
    public:
        static CUIManager* GetUIManager();
        
        void Init();
        void PreRenderUpdate();
        void PostRenderUpdate();
        void Destroy();

    private:
        void DescribeUIContent();
    };
}
