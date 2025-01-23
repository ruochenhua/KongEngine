#include "window.hpp"
#include "common.h"
#include <stdexcept>

using namespace Kong;

static KongWindow g_WindowModule;

KongWindow& KongWindow::GetWindowModule()
{
    return g_WindowModule;
}

KongWindow::KongWindow()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW3");
    }

    // 初始化opengl
    glfwWindowHint(GLFW_SAMPLES, 2);    // 抗锯齿
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_window = glfwCreateWindow(windowSize.x, windowSize.y, "Kong Sample", nullptr, nullptr);
    aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    if (m_window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create glfw window");
    }

    glfwMakeContextCurrent(m_window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        throw std::runtime_error("Failed to initialize GLAD");
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
