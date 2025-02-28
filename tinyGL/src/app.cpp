#include <chrono>

#include "App.hpp"

#include "Window.hpp"
#include "Render/GraphicsAPI/Vulkan/SimpleVulkanRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"

using namespace Kong;

constexpr double FRAME_TIME_CAP = 1.0/60.0;


KongApp::KongApp()
    : m_Window{KongWindow::GetWindowModule()}
#ifndef RENDER_IN_VULKAN
    , m_UIManager{KongUIManager::GetUIManager()}
#endif
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
#ifdef RENDER_IN_VULKAN
    string scene_name = "scene/hello_ssr.yaml";
    KongSceneManager::GetSceneManager().LoadScene(scene_name);
            
    SimpleVulkanRenderSystem simpleRenderSystem{m_renderer.GetSwapChainRenderPass()};
    simpleRenderSystem.CreateMeshDescriptorSet();
    
#endif
    
    double current_time = glfwGetTime();
    while (!glfwWindowShouldClose(m_Window.GetWindow()))
    {
        glfwPollEvents();
        double new_time = glfwGetTime();
        // // 在poll event之后，因为poll可能会pause（resize），需要记录这段时间的流逝
        // auto newTime = std::chrono::high_resolution_clock::now();
        // float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        // currentTime = newTime;
        //
        if (glfwGetKey(m_Window.GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_Window.GetWindow(), true);
            continue;
        }
        double delta = new_time - current_time;
        if(delta > FRAME_TIME_CAP)
#ifdef RENDER_IN_VULKAN
        {
            m_SceneManager.PreRenderUpdate(delta);
            
            m_renderer.BeginFrame();
            m_RenderModule.Update(delta);
            if (auto commandBuffer = m_renderer.GetCurrentCommandBuffer())
            {
                int frameIndex = m_renderer.GetFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    static_cast<float>(delta),
                    commandBuffer
                };

                simpleRenderSystem.UpdateMeshUBO(frameInfo);
                // render
                /* 每个frame之间可以有多个render pass，比如
                 * begin offscreen shadow pass
                 * render shadow casting object
                 * end offscreen shadow pass
                 * // reflection
                 * // post process
                 */
                // 在beginrenderpas之前就应该更新好UBO，在begin之后更新是不可靠的，数据可能会无法传递
                m_renderer.BeginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.RenderGameObjects(frameInfo);
                m_renderer.EndSwapChainRenderPass(commandBuffer);
                m_renderer.EndFrame();
            }

            current_time = new_time;
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
#ifdef RENDER_IN_VULKAN
    
    // cpu等待所有gpu任务完成
    vkDeviceWaitIdle(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice());
    
    // for (size_t i = 0; i < uboBuffers.size(); i++)
    // {
    //     uboBuffers[i] = nullptr;
    // }

    ResourceManager::Clean();

#endif
}
