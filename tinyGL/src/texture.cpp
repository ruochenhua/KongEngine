#include "Texture.hpp"
#include "common.h"

using namespace Kong;

void TextureBuilder::CreateTexture( GLuint& texture_id, const TextureCreateInfo& profile)
{
    // 如果这个id已经绑定了texture，先清理掉原来的
    if(texture_id)
    {
        glDeleteTextures(1, &texture_id);
        texture_id = GL_NONE;
    }
    
    auto tex_type = profile.texture_type;
    
    /////////// NO DSA
    glGenTextures(1, &texture_id);
    glBindTexture(tex_type, texture_id);
    
    if (tex_type == GL_TEXTURE_CUBE_MAP)
    {
        for (int i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, profile.internalFormat,
                profile.width, profile.height, 0, profile.format, profile.data_type, profile.data);    
        }
    }
    else if (tex_type == GL_TEXTURE_2D)
    {
        glTexImage2D(tex_type, 0, profile.internalFormat,
            profile.width, profile.height, 0, profile.format, profile.data_type, profile.data);
    }
    glTexParameteri(tex_type, GL_TEXTURE_WRAP_S, profile.wrapS);
    glTexParameteri(tex_type, GL_TEXTURE_WRAP_T, profile.wrapT);
    glTexParameteri(tex_type, GL_TEXTURE_WRAP_R, profile.wrapR);
    glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, profile.minFilter);
    glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, profile.magFilter);
    glGenerateMipmap(tex_type);
    
    glBindTexture(tex_type, 0);
    return;

    // 使用 DSA 创建纹理
    glCreateTextures(tex_type, 1, &texture_id);

    // 计算 Mipmap 级别
    int levels = static_cast<int>(std::log2(std::max(profile.width, profile.height))) + 1;

    // 分配纹理存储
    if (tex_type == GL_TEXTURE_CUBE_MAP) {
        glTextureStorage2D(texture_id, levels, profile.internalFormat, profile.width, profile.height);
        // for (int i = 0; i < 6; i++)
        // {
        //     glTextureSubImage2D(texture_id, 0, 0, 0, profile.width, profile.height,
        //                         profile.format, profile.data_type, profile.data);
        // }
    } else if (tex_type == GL_TEXTURE_2D) {
        glTextureStorage2D(texture_id, levels, profile.internalFormat, profile.width, profile.height);
        if (profile.data)
        {
            glTextureSubImage2D(texture_id, 0, 0, 0, profile.width, profile.height,
                                profile.format, profile.data_type, profile.data);
        }
    }

    // 设置纹理参数
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, profile.wrapS);
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, profile.wrapT);
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_R, profile.wrapR);
    glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, profile.minFilter);
    glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, profile.magFilter);

    // 生成 Mipmap
    glGenerateTextureMipmap(texture_id);
}

void TextureBuilder::CreateTexture2D(GLuint& texture_id, int width, int height, GLenum format, unsigned char* data)
{
    TextureCreateInfo texture_info {};
    texture_info.width = width;
    texture_info.height = height;
    texture_info.format = format;
    texture_info.data = data;

    // 默认的一些配置
    CreateTexture(texture_id, texture_info);
    return;

    //////////// DSA
    int levels = static_cast<int>(log2(max(width, height))) + 1;
    glCreateTextures(texture_info.texture_type, 1, &texture_id);
    
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, texture_info.wrapS);
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, texture_info.wrapT);
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_R, texture_info.wrapR);
    glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, texture_info.minFilter);
    glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, texture_info.magFilter);
    
    glTextureStorage2D(texture_id, levels, texture_info.internalFormat,
    static_cast<GLsizei>(texture_info.width),
    static_cast<GLsizei>(texture_info.height));
    
    if (data)
    {
        // 暂时还没有不同等级的区分
        // for (int level = 0; level < levels; level++)
        // {
        //     int mipWidth = std::max(1, width >> level);
        //     int mipHeight = std::max(1, height >> level);
        //     glTextureSubImage2D(texture_id, level, 0, 0, mipWidth, mipHeight,
        //         texture_info.format, texture_info.data_type, texture_info.data);    
        // }
        glTextureSubImage2D(texture_id, 0, 0, 0, width, height,
                texture_info.format, texture_info.data_type, texture_info.data);    
    }
    
    glGenerateTextureMipmap(texture_id);
}

void TextureBuilder::CreateTexture3D(GLuint& texture_id, const Texture3DCreateInfo& profile)
{
    // 如果这个id已经绑定了texture，先清理掉原来的
    if(texture_id)
    {
        glDeleteTextures(1, &texture_id);
        texture_id = GL_NONE;
    }
    
    auto tex_type = profile.texture_type;
    glGenTextures(1, &texture_id);
    glBindTexture(tex_type, texture_id);
    
    glTexImage3D(tex_type, 0, profile.internalFormat,
        profile.width, profile.height, profile.depth, 0,
        profile.format, profile.data_type, profile.data);

    glTexParameteri(tex_type, GL_TEXTURE_WRAP_S, profile.wrapS);
    glTexParameteri(tex_type, GL_TEXTURE_WRAP_T, profile.wrapT);
    glTexParameteri(tex_type, GL_TEXTURE_WRAP_R, profile.wrapR);
    glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, profile.minFilter);
    glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, profile.magFilter);

    glTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, profile.borderColor);
    
    glGenerateMipmap(tex_type);

    glBindTexture(tex_type, 0);
}


CTexture2D::CTexture2D(int width, int height, GLenum format, unsigned char* data)
{
#if 0
    glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
    
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTextureStorage2D(texture_id, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(texture_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    
    glGenerateTextureMipmap(texture_id);
        
#else
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
#endif
    
}
