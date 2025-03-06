#pragma once
#include "Render/RenderModule.hpp"
#include "ui.h"
#include "Window.hpp"

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
        KongSceneManager& m_SceneManager;
        
    };
}
