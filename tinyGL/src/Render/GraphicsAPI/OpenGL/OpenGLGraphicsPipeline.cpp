#include "OpenGLGraphicsPipeline.hpp"

#include "Render/Abstraction/IFrameContext.hpp"

#include "glad/glad.h"

namespace Kong
{
    OpenGLGraphicsPipeline::OpenGLGraphicsPipeline(GLuint program) : m_program(program) {}

    void OpenGLGraphicsPipeline::Bind(IFrameContext& frameContext)
    {
        (void)frameContext;
        if (m_program)
            glUseProgram(m_program);
    }

    void OpenGLGraphicsPipeline::BindGraphics(IRHICommandList* cmd)
    {
        (void)cmd;
        if (m_program)
            glUseProgram(m_program);
    }
}
