#include "RenderCommon.hpp"

#include <memory>

#include "RenderModule.hpp"
#ifdef RENDER_IN_VULKAN
#include "GraphicsAPI/Vulkan/VulkanRenderInfo.hpp"
#else
#include "glad/glad.h"
#endif

using namespace Kong;

void RenderMaterialInfo::BindTextureByType(ETextureType textureType, unsigned int location)
{
    auto texture = GetTextureByType(textureType);
    if (texture)
    {
        texture->Bind(location);
    }
#ifndef RENDER_IN_VULKAN
    else
    {
        auto nullTex = dynamic_cast<OpenGLTexture*>(KongRenderModule::GetNullTex());
        glBindTextureUnit(location, nullTex->GetTextureId());
    }
#endif
}

KongTexture* RenderMaterialInfo::GetTextureByType(ETextureType textureType)
{
    if (textures.find(textureType) != textures.end())
    {
        auto texture_ptr = textures[textureType].lock().get();
        if (texture_ptr && texture_ptr->IsValid())
        {
            return texture_ptr;
        }
    }
    
    return nullptr;
}

void RenderMaterialInfo::AddMaterialByType(ETextureType textureType, std::weak_ptr<KongTexture> texture)
{
    textures.emplace(textureType, texture);
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