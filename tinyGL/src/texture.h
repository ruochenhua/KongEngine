#pragma once
#include "glad/glad.h"

namespace Kong
{
    // 材质生成
    struct STextureProfile
    {
        GLenum texture_type = GL_TEXTURE_2D;
        GLenum internalFormat = GL_RGBA32F;
        GLenum format = GL_RGBA;

        GLenum data_type = GL_UNSIGNED_BYTE;
        int width = 1;
        int height = 1;

        GLint wrapS = GL_REPEAT;
        GLint wrapT = GL_REPEAT;
        GLint wrapR = GL_REPEAT;

        GLint minFilter = GL_NEAREST;
        GLint magFilter = GL_NEAREST;
    };
    class CTexture
    {
    public:
        // CTexture(int width, int height, GLenum format, unsigned char* data);
        // CTexture(const STextureProfile& profile, unsigned char* data);
        CTexture() = default;
        GLuint GetTextureId() const {return texture_id;}
        
    protected:
        GLuint texture_id = GL_NONE;
    };

    class CTexture2D : public CTexture
    {
    public:
        CTexture2D(int width, int height, GLenum format, unsigned char* data);
    };
}
