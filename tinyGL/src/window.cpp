#include <iostream>

#include "Window.hpp"
#include "common.h"
#include <stdexcept>
#include "Render/Abstraction/BackendType.hpp"
#include "Render/Abstraction/DeviceFactory.hpp"

using namespace Kong;

static KongWindow* g_WindowModule = nullptr;

KongWindow& KongWindow::GetWindowModule()
{
    if (g_WindowModule == nullptr)
    {
        g_WindowModule = new KongWindow();
    }
    return *g_WindowModule;
}

KongWindow::KongWindow()
{
#ifdef RENDER_IN_VULKAN
    BackendType backend = BackendType::Vulkan;
#else
    BackendType backend = BackendType::OpenGL;
#endif
    auto devicePtr = CreateGraphicsDevice(backend);
    m_device = devicePtr.get();
    m_window = static_cast<GLFWwindow*>(m_device->Init(windowSize.x, windowSize.y));
    aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW3");
    }
    
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetWindowUserPointer(m_window, this);   // 指定window的类型
    glfwSetWindowSizeCallback(m_window, frameBufferResizeCallback);
}

KongWindow::~KongWindow()
{
    std::cout << "Destroying window\n";
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
