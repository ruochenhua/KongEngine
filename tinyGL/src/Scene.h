#pragma once

#include "common.h"

namespace tinyGL
{
    class SceneLoader
    {
    public:
        static vector<SRenderInfo> LoadScene(const string& file_path);
    };
}
