#include "RenderCommon.hpp"

#include <memory>

#include "RenderModule.hpp"
#include "GraphicsAPI/GraphicsDevice.hpp"
#include "GraphicsAPI/OpenGL/OpenGLBuffer.hpp"
#include "GraphicsAPI/Vulkan/VulkanBuffer.hpp"

using namespace Kong;

void RenderMaterialInfo::BindTextureByType(ETextureType textureType, unsigned int location)
{
    auto texture = GetTextureByType(textureType);
    if (texture)
    {
        texture->Bind(location);
    }
    else
    {
        GLuint null_tex_id = KongRenderModule::GetNullTexId();
        glBindTextureUnit(location, null_tex_id);
    }
}

std::shared_ptr<KongTexture> RenderMaterialInfo::GetTextureByType(ETextureType textureType)
{
    if (textures.find(textureType) != textures.end() && textures[textureType]->IsValid())
    {
        return textures[textureType];
    }
    
    return nullptr;
}

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