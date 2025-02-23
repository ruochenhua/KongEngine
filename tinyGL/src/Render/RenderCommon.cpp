#include "RenderCommon.hpp"

#include <memory>

#include "GraphicsAPI/OpenGL/OpenGLBuffer.hpp"
#include "GraphicsAPI/Vulkan/VulkanBuffer.hpp"

using namespace Kong;

CMesh::CMesh()
#ifdef RENDER_IN_VULKAN
    : m_RenderInfo(std::make_unique<VulkanRenderInfo>())
#else
    : m_RenderInfo(std::make_unique<OpenGLRenderInfo>())
#endif
{
    
}

template <class T>
std::shared_ptr<T> RenderInfo::GetMaterial()
{
    return std::dynamic_pointer_cast<T>(material);
}