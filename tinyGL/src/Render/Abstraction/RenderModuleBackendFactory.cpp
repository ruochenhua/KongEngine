/**
 * @file RenderModuleBackendFactory.cpp
 * @brief CreateRenderModuleBackend 实现，按编译选项返回当前后端的 Backend 实例。
 * @ingroup RenderAbstraction
 */

#include "Render/Abstraction/IRenderModuleBackend.hpp"
#include "Render/Abstraction/BackendType.hpp"

#ifdef RENDER_IN_VULKAN
#include "Render/RenderModuleBackendVulkan.hpp"
#else
#include "Render/RenderModuleBackendOpenGL.hpp"
#endif

namespace Kong
{
    std::unique_ptr<IRenderModuleBackend> CreateRenderModuleBackend(BackendType type)
    {
#ifdef RENDER_IN_VULKAN
        if (type == BackendType::Vulkan)
            return std::make_unique<RenderModuleBackendVulkan>();
        return nullptr;
#else
        if (type == BackendType::OpenGL)
            return std::make_unique<RenderModuleBackendOpenGL>();
        return nullptr;
#endif
    }
}
