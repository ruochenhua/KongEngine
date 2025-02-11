#include "OpenGLGraphicsDevice.hpp"

#include <stdexcept>

using namespace Kong;

static OpenGLGraphicsDevice g_OpenGLDevice;

OpenGLGraphicsDevice* OpenGLGraphicsDevice::GetGraphicsDevice()
{
    return &g_OpenGLDevice;
}

OpenGLGraphicsDevice::OpenGLGraphicsDevice()
{
    m_API = GraphicsAPI::OPENGL;
}

OpenGLGraphicsDevice::~OpenGLGraphicsDevice()
{
}

GLFWwindow* OpenGLGraphicsDevice::Init(int width, int height)
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
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Kong Sample(OpenGL)", nullptr, nullptr);
    
    if (window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create glfw window");
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    return window;
}
