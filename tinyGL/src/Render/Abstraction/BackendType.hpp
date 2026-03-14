/**
 * @file BackendType.hpp
 * @brief RHI 后端类型枚举，与具体图形 API 解耦。
 * @ingroup RenderAbstraction
 */

#pragma once

namespace Kong
{
    /** 图形 API 后端类型，用于设备工厂与运行时选择 */
    enum class BackendType
    {
        OpenGL,
        Vulkan
    };
}
