#pragma once
#include "Common.h"

namespace Kong
{
    class AActor;
}

namespace Kong
{
    class CMeshComponent;
    class CSceneLoader
    {
    public:
        static bool LoadScene(const string& file_path, vector<shared_ptr<AActor>>& scene_actors);

        static string ToResourcePath(const string& in_path);

    };

    class KongSceneManager
    {
    public:
        static KongSceneManager& GetSceneManager();
        static vector<shared_ptr<AActor>> GetActors();
        static vector<weak_ptr<CMeshComponent>> GetMeshes();

        void PreRenderUpdate(double delta) const;
        void LoadScene(const string& file_path);
        
        vector<shared_ptr<AActor>> GetSceneActors_Implement();
        vector<weak_ptr<CMeshComponent>> GetSceneMeshes_Implement();
    private:
        vector<shared_ptr<AActor>> scene_actors;
        vector<weak_ptr<CMeshComponent>> scene_meshes;
    };
}
