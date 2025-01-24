#include "texture.h"
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
