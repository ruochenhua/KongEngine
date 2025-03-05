#pragma once
#include "Render/RenderModule.hpp"
#include "Scene.hpp"
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
#ifndef RENDER_IN_VULKAN
        KongUIManager& m_UIManager;
#endif
        KongRenderModule& m_RenderModule;
        KongSceneManager& m_SceneManager;
        
    };
}
