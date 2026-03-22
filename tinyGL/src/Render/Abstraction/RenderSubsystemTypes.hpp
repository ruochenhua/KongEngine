/**
 * @file RenderSubsystemTypes.hpp
 * @brief 与具体图形 API 解耦的子系统/Pass 查询枚举（原 OpenGLRenderSystem.hpp 内枚举迁出）。
 */

#pragma once

#include <cstdint>

namespace Kong
{
    enum class RenderSystemType : uint8_t
    {
        DEFERRED = 0,
        SKYBOX,
        POST_PROCESS,
        SS_REFLECTION,
        NONE,
    };
}
