#include "Scene.h"
#include "Utils.hpp"
#include "Component/Mesh/MeshComponent.h"

#include "Actor.h"
#include "Parser/YamlParser.h"

using namespace Kong;
KongSceneManager g_SceneManager;

string CSceneLoader::ToResourcePath(const string& in_path)
{
    return RESOURCE_PATH + in_path;
}


bool CSceneLoader::LoadScene(const string& file_path, vector<shared_ptr<AActor>>& scene_actors)
{
    std::string scene_file_content = Utils::ReadFile(ToResourcePath(file_path));
    auto yaml_file = file_path.find(".yaml");
    if(yaml_file != std::string::npos)
    {
        // 格式为yaml，读取yaml文件
        CYamlParser::ParseYamlFile(scene_file_content, scene_actors);
    }
    
    return true;
}

KongSceneManager& KongSceneManager::GetSceneManager()
{
    return g_SceneManager;
}

vector<shared_ptr<AActor>> KongSceneManager::GetActors()
{
    return g_SceneManager.GetSceneActors_Implement();
}

vector<weak_ptr<CMeshComponent>> KongSceneManager::GetMeshes()
{
    return g_SceneManager.GetSceneMeshes_Implement();
}

void KongSceneManager::PreRenderUpdate(double delta) const
{
    for(auto& actor : scene_actors)
    {
        actor->PreRenderUpdate(delta);
    }
}

vector<weak_ptr<CMeshComponent>> KongSceneManager::GetSceneMeshes_Implement()
{
    if(!g_SceneManager.scene_meshes.empty())
    {
        return g_SceneManager.scene_meshes;
    }
    
    auto actors = g_SceneManager.GetSceneActors_Implement();
    for(auto& actor : actors)
    {
        shared_ptr<CMeshComponent> mesh = actor->GetComponent<CMeshComponent>();
        if(!mesh)
        {
            continue;
        }

        g_SceneManager.scene_meshes.push_back(mesh);
    }
    
    return g_SceneManager.scene_meshes;
}

vector<shared_ptr<AActor>> KongSceneManager::GetSceneActors_Implement()
{
    return scene_actors;
}

void KongSceneManager::LoadScene(const string& file_path)
{
    for(auto& actor : scene_actors)
    {
        actor.reset();
    }
    scene_actors.clear();
    
    CSceneLoader::LoadScene(file_path, scene_actors);
    for(auto actor : scene_actors)
    {
        actor->BeginPlay();
    }
}
