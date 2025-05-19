#pragma once
#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>
#endif
#include "glad/glad.h"
#include "Render/RenderCommon.hpp"

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
        static std::shared_ptr<KongTexture> CreateTexture_new(const TextureCreateInfo& profile);
        static void CreateTexture2D(GLuint& texture_id, int width, int height, GLenum format, unsigned char* data = nullptr);
        static void CreateTexture3D(GLuint& texture_id, const Texture3DCreateInfo& profile);
    protected:
        GLuint texture_id = GL_NONE;
    };

    // opengl vulkan 分别处理
    class KongTexture
    {
    public:
        KongTexture() = default;
        virtual ~KongTexture() = default;
        virtual void CreateTexture(int width, int height, int nr_component, ETextureType textureType, unsigned char* pixels);
        virtual bool IsValid() = 0;

        virtual void Bind(unsigned int location) = 0;
    };

#ifdef RENDER_IN_VULKAN
    class VulkanTexture : public KongTexture
    {
    public:
        VulkanTexture() = default;
        VulkanTexture(const VkImageCreateInfo& profile);
        virtual ~VulkanTexture() override;
        bool IsValid() override;
        void CreateTexture(int width, int height, int nr_component, ETextureType textureType, unsigned char* pixels) override;
        void CreateCubemap(int width, int height, int nr_component, ETextureType textureType, unsigned char* pixels[6]);
        // 创建纹理图像视图, 为了让着色器能够访问纹理图像
        void CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags, int layerCount = 1);
        // 创建纹理采样器, 采样器定义了如何从纹理中采样颜色值
        void CreateTextureSampler();
        void CreateDepthTextureSampler();
        void Bind(unsigned int location) override;

        VkImage m_image{ nullptr };
        VkDeviceMemory m_memory{ nullptr };
        VkImageView m_imageView{ nullptr };
        VkSampler m_sampler{ nullptr };
        VkFormat m_format{ VK_FORMAT_MAX_ENUM };
        // 转换图像布局
        void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, int arrayLayer = 0);
        
    private:
        // 复制缓冲区到图像
        void CopyBufferToImage(VkBuffer buffer, VkImage image, int width, int height, int subresourceLayer = 0, int layerCount = 1);

    };
#endif
    class OpenGLTexture : public KongTexture
    {
    public:
        OpenGLTexture() = default;
        virtual ~OpenGLTexture() override;

        GLuint GetTextureId() const {return m_texId;}

        void Bind(unsigned int location) override;
        
        bool IsValid() override;
    private:
        friend class ResourceManager;
        friend class TextureBuilder;
        GLuint m_texId {GL_NONE};
    };
}
