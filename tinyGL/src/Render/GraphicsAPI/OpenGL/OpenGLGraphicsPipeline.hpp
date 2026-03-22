#pragma once

#include "Render/Abstraction/IPipeline.hpp"
#include "glad/glad.h"

namespace Kong
{
    /**
     * OpenGL 图形管线占位：绑定单个 glProgram。后续可扩展混合/深度/光栅状态对象。
     */
    class OpenGLGraphicsPipeline final : public IPipeline
    {
    public:
        explicit OpenGLGraphicsPipeline(GLuint program = 0);

        void SetProgram(GLuint program) { m_program = program; }
        GLuint GetProgram() const { return m_program; }

        void Bind(IFrameContext& frameContext) override;
        void BindGraphics(IRHICommandList* cmd) override;

    private:
        GLuint m_program {0};
    };
}
