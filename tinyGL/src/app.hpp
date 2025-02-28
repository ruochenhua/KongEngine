#pragma once
#include "Render/RenderModule.hpp"
#include "Scene.hpp"
#include "ui.h"
#include "Window.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanDescriptor.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanRenderer.hpp"

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

#ifdef RENDER_IN_VULKAN
        VulkanRenderer m_renderer;
#endif
    };
}
