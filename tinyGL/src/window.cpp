#include "Window.hpp"
#include "common.h"
#include <stdexcept>
#ifdef RENDER_IN_VULKAN
#include "Render/GraphicsAPI/Vulkan/VulkanGraphicsDevice.hpp"
#else
#include "Render/GraphicsAPI/OpenGL/OpenGLGraphicsDevice.hpp"
#endif

using namespace Kong;

static KongWindow g_WindowModule;

KongWindow& KongWindow::GetWindowModule()
{
    return g_WindowModule;
}

KongWindow::KongWindow()
{
#ifdef RENDER_IN_VULKAN
    auto graphics_device = VulkanGraphicsDevice::GetGraphicsDevice();
    m_window = graphics_device->Init(windowSize.x, windowSize.y);
    aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
     
#else
    auto graphics_device = OpenGLGraphicsDevice::GetGraphicsDevice();
    m_window = graphics_device->Init(windowSize.x, windowSize.y);
    aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    
#endif
    
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW3");
    }
    
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetWindowUserPointer(m_window, this);   // 指定window的类型
    glfwSetWindowSizeCallback(m_window, frameBufferResizeCallback);
}

GLFWwindow* KongWindow::GetWindow()
{
    return m_window;
}

void KongWindow::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto draw_window = reinterpret_cast<KongWindow*>(glfwGetWindowUserPointer(window));
    draw_window->windowSize = {width, height};
    draw_window->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    draw_window->resized = true;
}
