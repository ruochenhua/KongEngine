#include "Scene.h"
#include "Engine.h"
#include "Component/Mesh/MeshComponent.h"

#include "Actor.h"
#include "Parser/JsonParser.h"
#include "Parser/YamlParser.h"

using namespace Kong;
CScene* g_scene = new CScene;

string CSceneLoader::ToResourcePath(const string& in_path)
{
    return RESOURCE_PATH + in_path;
}


bool CSceneLoader::LoadScene(const string& file_path, vector<shared_ptr<AActor>>& scene_actors)
{
    std::string scene_file_content = Engine::ReadFile(ToResourcePath(file_path));
    auto json_file = file_path.find(".json");
    if(json_file != std::string::npos)
    {
        // 格式为json，读取json文件
        CJsonParser::ParseJsonFile(scene_file_content, scene_actors);
    }
    
    auto yaml_file = file_path.find(".yaml");
    if(yaml_file != std::string::npos)
    {
        // 格式为yaml，读取yaml文件
        CYamlParser::ParseYamlFile(scene_file_content, scene_actors);
    }
    
    return true;
}

CScene* CScene::GetScene()
{
    return g_scene;
}

vector<shared_ptr<AActor>> CScene::GetActors()
{
    return g_scene->GetSceneActors_Implement();
}

vector<weak_ptr<CMeshComponent>> CScene::GetMeshes()
{
    return g_scene->GetSceneMeshes_Implement();
}

void CScene::UpdateScene(double delta) const
{
    for(auto& actor : scene_actors)
    {
        actor->Update(delta);
    }
}

vector<weak_ptr<CMeshComponent>> CScene::GetSceneMeshes_Implement()
{
    if(!g_scene->scene_meshes.empty())
    {
        return g_scene->scene_meshes;
    }
    
    auto actors = g_scene->GetSceneActors_Implement();
    for(auto& actor : actors)
    {
        shared_ptr<CMeshComponent> mesh = actor->GetComponent<CMeshComponent>();
        if(!mesh)
        {
            continue;
        }

        g_scene->scene_meshes.push_back(mesh);
    }
    
    return g_scene->scene_meshes;
}

vector<shared_ptr<AActor>> CScene::GetSceneActors_Implement()
{
    return scene_actors;
}

void CScene::LoadScene(const string& file_path)
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
