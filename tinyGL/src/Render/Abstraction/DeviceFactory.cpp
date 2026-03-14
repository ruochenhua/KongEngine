/**
 * @file DeviceFactory.cpp
 * @brief RHI 设备工厂实现，编译期选择 OpenGL/Vulkan。
 * @ingroup RenderAbstraction
 */

#include "Render/Abstraction/DeviceFactory.hpp"
#include "Render/Abstraction/BackendType.hpp"

#ifdef RENDER_IN_VULKAN
#include "Render/GraphicsAPI/Vulkan/VulkanGraphicsDevice.hpp"
#else
#include "Render/GraphicsAPI/OpenGL/OpenGLGraphicsDevice.hpp"
#endif

namespace Kong
{
    static void NoOpDeviceDeleter(IGraphicsDevice*) {}

    GraphicsDevicePtr CreateGraphicsDevice(BackendType type)
    {
#ifdef RENDER_IN_VULKAN
        if (type == BackendType::Vulkan)
        {
            std::shared_ptr<VulkanGraphicsDevice> vk = VulkanGraphicsDevice::GetGraphicsDevice();
            return GraphicsDevicePtr(vk.get(), &NoOpDeviceDeleter);
        }
#else
        if (type == BackendType::OpenGL)
        {
            OpenGLGraphicsDevice* gl = OpenGLGraphicsDevice::GetGraphicsDevice();
            return GraphicsDevicePtr(gl, &NoOpDeviceDeleter);
        }
#endif
        return GraphicsDevicePtr(nullptr, &NoOpDeviceDeleter);
    }
}
