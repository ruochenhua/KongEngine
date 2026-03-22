#include "UBOHelper.hpp"

namespace Kong
{
    void UBOHelper::Init(GLuint in_binding)
    {
        binding = in_binding;
        glGenBuffers(1, &ubo_idx);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_idx);
        glBufferData(GL_UNIFORM_BUFFER, next_offset, NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_idx);
        glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
    }

    void UBOHelper::Bind() const
    {
#if !USE_DSA
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_idx);
#endif
    }

    void UBOHelper::EndBind() const
    {
#if !USE_DSA
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif
    }
}
