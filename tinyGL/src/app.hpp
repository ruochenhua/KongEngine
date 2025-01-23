#pragma once
#include "render.h"
#include "ui.h"
#include "window.hpp"

namespace Kong
{
    class KongApp
    {
    public:
        KongApp();
        ~KongApp();

        KongApp(const KongApp& other) = delete;
        KongApp& operator=(const KongApp& other) = delete;
        
        void Run();
        
    private:
        KongWindow& m_Window;
        KongUIManager& m_UIManager;
        KongRenderModule& m_RenderModule;
    };
}
