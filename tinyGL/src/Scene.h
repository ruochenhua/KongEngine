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

        static string ToResourcePath(const string& file_path);

    };

    class CScene
    {
    public:
        static CScene* GetScene();

        void LoadScene(const string& file_path);
        const vector<shared_ptr<CRenderObj>>& GetSceneRenderObjects() const;
        const vector<shared_ptr<Light>>& GetSceneLights() const;
    private:
        vector<shared_ptr<CRenderObj>> render_objs;
        vector<shared_ptr<Light>> lights;
    };
}
