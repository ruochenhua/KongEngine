/**
 * @file Types.hpp
 * @brief RHI 层与 API 无关的枚举与描述符类型，不依赖 OpenGL/Vulkan。
 * @ingroup RenderAbstraction
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace Kong
{
    // ========== 数据格式与用途枚举 ==========

    /** 像素/顶点数据格式，实现层映射到 GL_* / VkFormat */
    enum class DataFormat : uint32_t
    {
        Unknown = 0,
        R8G8B8A8_UNORM,
        R8G8B8A8_SRGB,
        R32G32B32A32_SFLOAT,
        R32G32B32_SFLOAT,
        D24_UNORM_S8_UINT,
        D32_SFLOAT,
        D32_SFLOAT_S8_UINT,
    };

    /** 纹理用途位掩码 */
    enum TextureUsage : uint32_t
    {
        TextureUsage_None         = 0,
        TextureUsage_Sampled       = 1 << 0,
        TextureUsage_ColorAttachment = 1 << 1,
        TextureUsage_DepthStencil = 1 << 2,
        TextureUsage_TransferDst   = 1 << 3,
        TextureUsage_TransferSrc   = 1 << 4,
    };

    /** 缓冲用途 */
    enum class BufferUsage : uint32_t
    {
        Vertex   = 0,
        Index,
        Uniform,
        Staging,
    };

    /** 纹理过滤 */
    enum class TextureFilter : uint32_t
    {
        Nearest,
        Linear,
        LinearMipmapNearest,
        LinearMipmapLinear,
    };

    /** 纹理环绕 */
    enum class TextureWrap : uint32_t
    {
        Repeat,
        ClampToEdge,
        ClampToBorder,
        MirroredRepeat,
    };

    /** 着色器阶段（引擎侧），实现层映射到 GL_*_SHADER / VkShaderStageFlagBits */
    enum class ShaderStage : uint32_t
    {
        Vertex   = 0,
        Fragment = 1,
        Geometry = 2,
        Compute  = 3,
        TessControl = 4,
        TessEvaluation = 5,
    };

    // ========== 描述符结构体（仅字段，无 GL/Vk 类型） ==========

    /** 缓冲创建描述符 */
    struct BufferDesc
    {
        BufferUsage usage     = BufferUsage::Vertex;
        uint64_t    size      = 0;
        uint32_t    instanceCount = 1;
        void*       initialData = nullptr;  ///< 可选初始数据
    };

    /** 纹理创建描述符 */
    struct TextureDesc
    {
        int          width       = 1;
        int          height      = 1;
        int          depth       = 1;       ///< 3D 或 layer 数
        DataFormat   format      = DataFormat::R8G8B8A8_UNORM;
        uint32_t     usage       = TextureUsage_Sampled;
        TextureFilter minFilter  = TextureFilter::Linear;
        TextureFilter magFilter  = TextureFilter::Linear;
        TextureWrap   wrapS      = TextureWrap::Repeat;
        TextureWrap   wrapT      = TextureWrap::Repeat;
        TextureWrap   wrapR      = TextureWrap::Repeat;
        void*         initialData = nullptr;
    };

    /** 管线创建描述符（仅字段占位，阶段 5 细化） */
    struct PipelineDesc
    {
        std::vector<std::pair<uint32_t, std::string>> shaderStages;  ///< (ShaderStage 枚举, 路径或标识)
        // 顶点布局、混合、深度等由实现层扩展或后续补充
    };

    /** 渲染通道描述符（仅字段占位，阶段 5 细化） */
    struct RenderPassDesc
    {
        uint32_t colorAttachmentCount = 1;
        bool     hasDepthStencil      = true;
        // load/store 等由实现层扩展
    };

    // ========== 场景绘制信息（与 API 无关，供 IRenderSystem::Draw 使用） ==========

    /**
     * 单帧绘制上下文数据，由 RenderModule 填充，各 Pass 只读或更新 RT 句柄。
     * 当前 RT 句柄为不透明指针，实现层可转为 ITexture* 或后端句柄。
     */
    struct SceneDrawInfo
    {
        int       frameIndex   = 0;
        float     frameTime    = 0.0f;
        void*     sceneContext = nullptr;   ///< 引擎场景/光照等，实现层按需强转
        uintptr_t currentColorRT = 0;       ///< 当前主颜色 RT 句柄（占位，后续可改为 ITexture*）
        uintptr_t currentDepthRT = 0;       ///< 当前深度 RT 句柄
    };
}
