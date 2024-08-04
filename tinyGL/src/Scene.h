#pragma once
#include "Common.h"

namespace tinyGL
{
    class AActor;
}

namespace tinyGL
{
    class CLightComponent;
    class CMeshComponent;
    class CSceneLoader
    {
    public:
        static bool LoadScene(const string& file_path, vector<shared_ptr<AActor>>& scene_actors);

        static string ToResourcePath(const string& file_path);

    };

    class CScene
    {
    public:
        static CScene* GetScene();
        static vector<shared_ptr<AActor>> GetActors();
        static vector<weak_ptr<CMeshComponent>> GetMeshes();
        
        void LoadScene(const string& file_path);
        vector<shared_ptr<AActor>> GetSceneActors_Implement();
        vector<weak_ptr<CMeshComponent>> GetSceneMeshes_Implement();
    private:
        vector<shared_ptr<AActor>> scene_actors;
        vector<weak_ptr<CMeshComponent>> scene_meshes;
    };
}
