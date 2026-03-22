#pragma once

#include "Common.h"
#include <map>
#include <string>

namespace Kong
{
    /**
     * OpenGL UBO 辅助；仅由 OpenGLRenderPassHost 等 GL 层使用，不属于 KongRenderModule 公开头。
     */
    class UBOHelper
    {
    public:
        template <class T>
        void AppendData(T data, const std::string& name);

        template <class T>
        void UpdateData(const T& data, const std::string& name) const;

        void Init(GLuint in_binding);
        void Bind() const;
        void EndBind() const;

    private:
        std::map<string, unsigned> data_offset_cache;
        size_t     next_offset = 0;
        GLuint     binding     = GL_NONE;
        GLuint     ubo_idx     = GL_NONE;
    };

    template <class T>
    void UBOHelper::AppendData(T data, const std::string& name)
    {
        data_offset_cache.emplace(name, next_offset);
        size_t size = sizeof(T);
        next_offset += size;
    }

#if USE_DSA
    template <class T>
    void UBOHelper::UpdateData(const T& data, const std::string& name) const
    {
        auto find_iter = data_offset_cache.find(name);
        if (find_iter == data_offset_cache.end())
        {
            assert(false, "update data failed");
            return;
        }
        unsigned offset = find_iter->second;
        size_t   size   = sizeof(T);
        glNamedBufferSubData(ubo_idx, offset, size, &data);
    }
#else
    template <class T>
    void UBOHelper::UpdateData(const T& data, const std::string& name) const
    {
        auto find_iter = data_offset_cache.find(name);
        if (find_iter == data_offset_cache.end())
        {
            assert(false, "update data failed");
            return;
        }
        unsigned offset = find_iter->second;
        size_t   size   = sizeof(T);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &data);
    }
#endif
}
