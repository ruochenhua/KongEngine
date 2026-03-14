/**
 * @file GLTexture.cpp
 * @brief OpenGL ITexture 实现。
 * @ingroup RenderAbstraction
 */

#include "GLTexture.hpp"
#include "glad/glad.h"
#include <stdexcept>

namespace Kong
{
    static void ToGLFormat(DataFormat fmt, unsigned int* outInternal, unsigned int* outFormat, unsigned int* outType)
    {
        switch (fmt)
        {
        case DataFormat::R8G8B8A8_UNORM:
            *outInternal = 0x8058; /* GL_RGBA8 */
            *outFormat = 0x1908;   /* GL_RGBA */
            *outType = 0x1401;    /* GL_UNSIGNED_BYTE */
            break;
        case DataFormat::R8G8B8A8_SRGB:
            *outInternal = 0x8C43; /* GL_SRGB8_ALPHA8 */
            *outFormat = 0x1908;
            *outType = 0x1401;
            break;
        case DataFormat::R32G32B32A32_SFLOAT:
            *outInternal = 0x8814; /* GL_RGBA32F */
            *outFormat = 0x1908;
            *outType = 0x1406;    /* GL_FLOAT */
            break;
        case DataFormat::R32G32B32_SFLOAT:
            *outInternal = 0x8815; /* GL_RGB32F */
            *outFormat = 0x1907;  /* GL_RGB */
            *outType = 0x1406;
            break;
        case DataFormat::D24_UNORM_S8_UINT:
            *outInternal = 0x84F9; /* GL_DEPTH24_STENCIL8 */
            *outFormat = 0x84F9;
            *outType = 0x84FA;    /* GL_UNSIGNED_INT_24_8 */
            break;
        case DataFormat::D32_SFLOAT:
            *outInternal = 0x8CAC; /* GL_DEPTH_COMPONENT32F */
            *outFormat = 0x1902;   /* GL_DEPTH_COMPONENT */
            *outType = 0x1406;
            break;
        default:
            *outInternal = 0x8058;
            *outFormat = 0x1908;
            *outType = 0x1401;
            break;
        }
    }

    static unsigned int ToGLWrap(TextureWrap w)
    {
        switch (w)
        {
        case TextureWrap::Repeat:         return 0x2901; /* GL_REPEAT */
        case TextureWrap::ClampToEdge:    return 0x812F; /* GL_CLAMP_TO_EDGE */
        case TextureWrap::ClampToBorder:  return 0x812D; /* GL_CLAMP_TO_BORDER */
        case TextureWrap::MirroredRepeat: return 0x8370; /* GL_MIRRORED_REPEAT */
        default: return 0x2901;
        }
    }

    static unsigned int ToGLFilter(TextureFilter f)
    {
        switch (f)
        {
        case TextureFilter::Nearest: return 0x2600; /* GL_NEAREST */
        case TextureFilter::Linear:  return 0x2601; /* GL_LINEAR */
        case TextureFilter::LinearMipmapNearest: return 0x2701; /* GL_LINEAR_MIPMAP_NEAREST */
        case TextureFilter::LinearMipmapLinear:  return 0x2703; /* GL_LINEAR_MIPMAP_LINEAR */
        default: return 0x2601;
        }
    }

    GLTexture::GLTexture(const TextureDesc& desc)
        : m_width(desc.width > 0 ? desc.width : 1)
        , m_height(desc.height > 0 ? desc.height : 1)
        , m_depth(desc.depth > 0 ? desc.depth : 1)
    {
        unsigned int internalFormat, format, type;
        ToGLFormat(desc.format, &internalFormat, &format, &type);

        glGenTextures(1, &m_texId);
        glBindTexture(0x0DE1, m_texId); /* GL_TEXTURE_2D */
        glTexImage2D(0x0DE1, 0, internalFormat, m_width, m_height, 0, format, type, desc.initialData);
        glTexParameteri(0x0DE1, 0x2801, ToGLFilter(desc.minFilter)); /* GL_TEXTURE_MIN_FILTER */
        glTexParameteri(0x0DE1, 0x2800, ToGLFilter(desc.magFilter)); /* GL_TEXTURE_MAG_FILTER */
        glTexParameteri(0x0DE1, 0x2802, ToGLWrap(desc.wrapS));       /* GL_TEXTURE_WRAP_S */
        glTexParameteri(0x0DE1, 0x2803, ToGLWrap(desc.wrapT));     /* GL_TEXTURE_WRAP_T */
        glBindTexture(0x0DE1, 0);
    }

    GLTexture::~GLTexture()
    {
        if (m_texId)
        {
            glDeleteTextures(1, &m_texId);
            m_texId = 0;
        }
    }

    void GLTexture::Bind(uint32_t slot, void* commandList)
    {
        (void)commandList;
        glActiveTexture(0x84C0 + slot); /* GL_TEXTURE0 */
        glBindTexture(0x0DE1, m_texId);
    }
}
