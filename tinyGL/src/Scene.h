#pragma once
#include "common.h"

namespace tinyGL
{
    class CRenderObj;
    class CSceneLoader
    {
    public:
        static vector<shared_ptr<CRenderObj>> LoadScene(const string& file_path);

    };
}
