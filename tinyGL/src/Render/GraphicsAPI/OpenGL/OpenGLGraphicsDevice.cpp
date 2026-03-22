#include "OpenGLGraphicsDevice.hpp"
#include "OpenGLRenderPassHost.hpp"
#include "GLFW/glfw3.h"
#include <stdexcept>

struct GLFWwindow;

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

void* OpenGLGraphicsDevice::Init(int width, int height)
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

    m_window = window;
    return static_cast<void*>(window);
}

IFrameContext& OpenGLGraphicsDevice::BeginFrame()
{
    return m_frameContext;
}

void OpenGLGraphicsDevice::EndFrame()
{
    if (m_window)
        glfwSwapBuffers(m_window);
}

std::unique_ptr<IBuffer> OpenGLGraphicsDevice::CreateBuffer(const BufferDesc& desc)
{
    return std::make_unique<GLBuffer>(desc);
}

std::unique_ptr<ITexture> OpenGLGraphicsDevice::CreateTexture(const TextureDesc& desc)
{
    return std::make_unique<GLTexture>(desc);
}

std::unique_ptr<IRenderPassHost> OpenGLGraphicsDevice::CreateRenderPassHost()
{
    return std::make_unique<OpenGLRenderPassHost>();
}
