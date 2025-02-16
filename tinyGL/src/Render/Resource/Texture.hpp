#pragma once
#include <string>

#include "glad/glad.h"

namespace Kong
{
    // 材质生成
    struct TextureCreateInfo
    {
        GLenum texture_type { GL_TEXTURE_2D };
        GLint internalFormat { GL_RGBA16F };
        GLenum format { GL_RGBA };

        GLenum data_type { GL_UNSIGNED_BYTE };
        int width { 1 };
        int height { 1 };

        GLint wrapS { GL_REPEAT };
        GLint wrapT { GL_REPEAT };
        GLint wrapR { GL_REPEAT };

        GLint minFilter { GL_LINEAR_MIPMAP_NEAREST };
        GLint magFilter { GL_NEAREST };

        void* data { nullptr };

    };

    struct Texture3DCreateInfo : public TextureCreateInfo
    {
        int depth { 1 };
        // 先放这
        GLfloat borderColor[4] = { 1.f, 1.f, 1.f, 1.f };
    };
    
    class TextureBuilder
    {
    public:
        // CTexture(int width, int height, GLenum format, unsigned char* data);
        // CTexture(const STextureProfile& profile, unsigned char* data);
        TextureBuilder() = default;
        GLuint GetTextureId() const {return texture_id;}

        static void CreateTexture(GLuint& texture_id, const TextureCreateInfo& profile);
        static void CreateTexture2D(GLuint& texture_id, int width, int height, GLenum format, unsigned char* data = nullptr);
        static void CreateTexture3D(GLuint& texture_id, const Texture3DCreateInfo& profile);
    protected:
        GLuint texture_id = GL_NONE;
    };

    // opengl vulkan 分别处理
    class Texture
    {
    public:
        
    };
    
}
