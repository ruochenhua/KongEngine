#include "app.hpp"

#include "window.hpp"

using namespace Kong;

constexpr double FRAME_TIME_CAP = 1.0/200.0;

KongApp::KongApp()
    : m_Window{KongWindow::GetWindowModule()}
    , m_UIManager{KongUIManager::GetUIManager()}
    , m_RenderModule{KongRenderModule::GetRenderModule()}
{
    m_UIManager.Init(m_Window.GetWindow());
    m_RenderModule.Init();
}

KongApp::~KongApp()
{
    m_UIManager.Destroy();
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
        {
            m_UIManager.PreRenderUpdate(delta);
            // CScene::GetScene()->PreRenderUpdate(delta);
            m_RenderModule.Update(delta);
			
            m_UIManager.PostRenderUpdate();
            
            m_RenderModule.PostUpdate();
            
            current_time = new_time;
        }
    }
}
