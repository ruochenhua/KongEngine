#include "App.hpp"
#include "Scene.hpp"
#include "Window.hpp"

using namespace Kong;

constexpr double FRAME_TIME_CAP = 1.0/60.0;


KongApp::KongApp()
    : m_Window{KongWindow::GetWindowModule()}
    , m_UIManager{KongUIManager::GetUIManager()}
    , m_RenderModule{KongRenderModule::GetRenderModule()}
    , m_SceneManager{KongSceneManager::GetSceneManager()}
{
    m_RenderModule.Init();
    // 需要rendermodule先初始化完，拿到descriptorpool等信息
    m_UIManager.Init(m_Window.GetWindow());
}

KongApp::~KongApp()
{
#ifdef RENDER_IN_VULKAN
    // cpu等待所有gpu任务完成
    vkDeviceWaitIdle(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice());
    ResourceManager::Clean();
#endif
    
    m_UIManager.Destroy();
}

void KongApp::Run()
{
    // 直接加载测试场景
    KongSceneManager::GetSceneManager().LoadScene("scene/hello_ssr.yaml");
    double current_time = glfwGetTime();
    while (!glfwWindowShouldClose(m_Window.GetWindow()))
    {
        glfwPollEvents();
        double new_time = glfwGetTime();
        // 在poll event之后，因为poll可能会pause（resize），需要记录这段时间的流逝
        if (glfwGetKey(m_Window.GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_Window.GetWindow(), true);
            continue;
        }
        double delta = new_time - current_time;
        if(delta > FRAME_TIME_CAP)
        {
            m_UIManager.PreRenderUpdate(delta);
            m_SceneManager.PreRenderUpdate(delta);
            
#ifdef RENDER_IN_VULKAN
            m_RenderModule.BeginFrame();
#endif

            m_RenderModule.Update(delta);
      
            m_UIManager.PostRenderUpdate();
#ifdef RENDER_IN_VULKAN      
            m_RenderModule.EndFrame();
#else
            glfwSwapBuffers(m_Window.GetWindow());
#endif
            current_time = new_time;
        }
    }
}
