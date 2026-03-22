/**
 * @file GLTexture.hpp
 * @brief OpenGL 实现的 ITexture，RHI 纹理接口。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/ITexture.hpp"
#include "Render/Abstraction/Types.hpp"
#include <cstdint>

namespace Kong
{
    /** OpenGL 后端 ITexture 实现 */
    class GLTexture : public ITexture
    {
    public:
        explicit GLTexture(const TextureDesc& desc);
        ~GLTexture() override;

        int  GetWidth()  const override { return m_width; }
        int  GetHeight() const override { return m_height; }
        int  GetDepth()  const override { return m_depth; }
        void Bind(uint32_t slot, void* commandList = nullptr) override;

        unsigned int GetGLTextureId() const { return m_texId; }

    private:
        unsigned int m_texId {0};
        int          m_width {1};
        int          m_height {1};
        int          m_depth {1};
    };

    /**
     * 包装已存在的 GL 纹理名（不拥有、不 glDelete），用于把引擎遗留 GLuint 接到 ITexture / IFramebuffer。
     */
    class GLTextureView : public ITexture
    {
    public:
        GLTextureView(unsigned int glTextureId, int width, int height);
        ~GLTextureView() override = default;

        int  GetWidth() const override { return m_width; }
        int  GetHeight() const override { return m_height; }
        void Bind(uint32_t slot, void* commandList = nullptr) override;

        unsigned int GetGLTextureId() const { return m_texId; }

    private:
        unsigned int m_texId {0};
        int          m_width {1};
        int          m_height {1};
    };
}
