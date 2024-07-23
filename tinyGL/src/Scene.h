#pragma once
#include "common.h"

namespace tinyGL
{
    class Light;
    class CRenderObj;
    class CSceneLoader
    {
    public:
        static bool LoadScene(const string& file_path,
            vector<shared_ptr<CRenderObj>>& render_objs,
            vector<shared_ptr<Light>>& lights);

    };
}
