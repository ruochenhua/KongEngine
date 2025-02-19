#include <chrono>

#include "App.hpp"

#include "Window.hpp"
#include "Render/GraphicsAPI/Vulkan/SimpleVulkanRenderSystem.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanSwapChain.hpp"

using namespace Kong;

constexpr double FRAME_TIME_CAP = 1.0/60.0;
struct GlobalUbo
{
    glm::mat4 projectionView {1.};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1., -3., -1.});
};

KongApp::KongApp()
    : m_Window{KongWindow::GetWindowModule()}
#ifndef RENDER_IN_VULKAN
    , m_UIManager{KongUIManager::GetUIManager()}
#endif
    , m_RenderModule{KongRenderModule::GetRenderModule()}
    , m_SceneManager{KongSceneManager::GetSceneManager()}
{
#ifdef RENDER_IN_VULKAN
    m_globalPool = VulkanDescriptorPool::Builder()
                    .SetMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .Build();
    
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
    std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++)
    {
        uboBuffers[i] = std::make_unique<VulkanBuffer>();
        uboBuffers[i]->Initialize(UNIFORM_BUFFER, sizeof(GlobalUbo),1);
        uboBuffers[i]->Map();
    }
    
    auto globalSetLayout = VulkanDescriptorSetLayout::Builder()
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
            
    std::vector<VkDescriptorSet> globalDiscriptorSets{VulkanSwapChain::MAX_FRAMES_IN_FLIGHT};
    for (int i = 0; i < globalDiscriptorSets.size(); i++)
    {
        auto bufferInfo = uboBuffers[i]->DescriptorInfo();
        VulkanDescriptorWriter(*globalSetLayout, *m_globalPool)
        .WriteBuffer(0, &bufferInfo)
        .Build(globalDiscriptorSets[i]);
    }
            
    SimpleVulkanRenderSystem simpleRenderSystem{m_renderer.GetSwapChainRenderPass(),
        globalSetLayout->GetDescriptorSetLayout()};
    
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
            m_RenderModule.Update(delta);
            
            m_renderer.BeginFrame();
            if (auto commandBuffer = m_renderer.GetCurrentCommandBuffer())
            {
                int frameIndex = m_renderer.GetFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    static_cast<float>(delta),
                    commandBuffer,
                    globalDiscriptorSets[frameIndex]
                };

                // 更新ubo数据
                GlobalUbo ubo{};
                ubo.projectionView = m_RenderModule.GetCamera()->GetProjectionMatrix() * m_RenderModule.GetCamera()->GetViewMatrix();
                // globalUboBuffer.writeToBuffer(&ubo, frameIndex);
                // globalUboBuffer.flushIndex(frameIndex);
                uboBuffers[frameIndex]->WriteToBuffer(&ubo);
                uboBuffers[frameIndex]->Flush();
                
                // render
                /* 每个frame之间可以有多个render pass，比如
                 * begin offscreen shadow pass
                 * render shadow casting object
                 * end offscreen shadow pass
                 * // reflection
                 * // post process
                 */
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
    
    for (size_t i = 0; i < uboBuffers.size(); i++)
    {
        uboBuffers[i] = nullptr;
    }

    ResourceManager::Clean();

#endif
}
