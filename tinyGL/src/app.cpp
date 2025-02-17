#include "App.hpp"

#include "Window.hpp"

using namespace Kong;

constexpr double FRAME_TIME_CAP = 1.0/200.0;

KongApp::KongApp()
    : m_Window{KongWindow::GetWindowModule()}
    , m_UIManager{KongUIManager::GetUIManager()}
    , m_RenderModule{KongRenderModule::GetRenderModule()}
    , m_SceneManager{KongSceneManager::GetSceneManager()}
{
#ifdef RENDER_IN_VULKAN
#else
    m_UIManager.Init(m_Window.GetWindow());
#endif
    m_RenderModule.Init();
}

KongApp::~KongApp()
{
#ifdef RENDER_IN_VULKAN
#else
    m_UIManager.Destroy();
#endif
}

void KongApp::Run()
{
    double current_time = glfwGetTime();
    while (!glfwWindowShouldClose(m_Window.GetWindow()))
    {
        glfwPollEvents();
        double new_time = glfwGetTime();
        
        if (glfwGetKey(m_Window.GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_Window.GetWindow(), true);
            continue;
        }
        double delta = new_time - current_time;
        if(delta > FRAME_TIME_CAP)
#ifdef RENDER_IN_VULKAN
        {
            
        }
#else
        {
            m_UIManager.PreRenderUpdate(delta);
            m_SceneManager.PreRenderUpdate(delta);
            
            m_RenderModule.Update(delta);
			
            m_UIManager.PostRenderUpdate();
            
            glfwSwapBuffers(m_Window.GetWindow());

            current_time = new_time;
        }
#endif       
    }
}
