#include "Texture.hpp"

#include <yaml-cpp/node/detail/memory.h>

#include "common.h"
#include "Render/GraphicsAPI/Vulkan/VulkanGraphicsDevice.hpp"

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

#ifdef RENDER_IN_VULKAN

VulkanTexture::~VulkanTexture()
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroyImage(device, m_image, nullptr);
    vkDestroyImageView(device, m_imageView, nullptr);
    vkFreeMemory(device, m_memory, nullptr);
}

bool VulkanTexture::IsValid()
{
     return m_memory && m_image && m_imageView;
}

void VulkanTexture::CreateTexture(int width, int height, int nr_component, unsigned char* pixels)
{
// todo: 这里放到vulkanbuffer里面
    // 创建纹理和内存
    VkDeviceSize imageSize = width * height * nr_component;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    auto device = VulkanGraphicsDevice::GetGraphicsDevice();
    device->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    switch (nr_component)
    {
    case 1:
        m_format = VK_FORMAT_R8_SRGB;
        break;
    case 2:
        m_format = VK_FORMAT_R8G8_SRGB;
        break;
    case 3:
        // vulkan对rgb8的支持不足，大多数GPU设备都不支持，先屏蔽掉这个
        m_format = VK_FORMAT_R8G8B8_UNORM;
        break;
    case 4:
        m_format = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    }
    void* data;
    vkMapMemory(device->GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(device->GetDevice(), stagingBufferMemory);


    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    
    device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);

    TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer, m_image, width, height);
    TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device->GetDevice(), stagingBufferMemory, nullptr);

    // 创建纹理图像imageview
    m_imageView = CreateImageView(m_image, m_format, VK_IMAGE_ASPECT_COLOR_BIT);

    // 创建采样器
    CreateTextureSampler();
}



void VulkanTexture::Bind(unsigned int location)
{
    
}

void VulkanTexture::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = VulkanGraphicsDevice::GetGraphicsDevice()->BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VulkanGraphicsDevice::GetGraphicsDevice()->EndSingleTimeCommands(commandBuffer);
}

void VulkanTexture::CopyBufferToImage(VkBuffer buffer, VkImage image, int width, int height)
{
    VkCommandBuffer commandBuffer = VulkanGraphicsDevice::GetGraphicsDevice()->BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    VulkanGraphicsDevice::GetGraphicsDevice()->EndSingleTimeCommands(commandBuffer);
}

VkImageView VulkanTexture::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void VulkanTexture::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

#else


OpenGLTexture::~OpenGLTexture()
{
    if (m_texId)
    {
        glDeleteTextures(1, &m_texId);
        m_texId = GL_NONE;
    }
}

void OpenGLTexture::Bind(unsigned int location)
{
    glBindTextureUnit(location, m_texId);
}

bool OpenGLTexture::IsValid()
{
    return m_texId != GL_NONE;
}

void OpenGLTexture::LoadTexture(const std::string& fileName)
{
    
}

#endif


