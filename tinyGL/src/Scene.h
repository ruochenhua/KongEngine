#pragma once
#include "common.h"

namespace tinyGL
{
    class CSceneLoader
    {
    public:
        static vector<SRenderInfo> LoadScene(const string& file_path);

    };
}
