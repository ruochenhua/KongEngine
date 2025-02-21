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
