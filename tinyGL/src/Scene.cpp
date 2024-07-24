#include "Scene.h"
#include "Engine.h"
#include "light.h"
#include "model.h"
#include "utilityshape.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace tinyGL;

string ToResourcePath(const string& in_path)
{
    return RESOURCE_PATH + in_path;
}

glm::vec3 ParseVec3(const vector<float>& in_vector)
{
    assert(in_vector.size() == 3);
    return glm::vec3(in_vector[0], in_vector[1], in_vector[2]);
}

void ParseTransform(nlohmann::basic_json<> in_json, shared_ptr<SceneObject> scene_object)
{
    if(in_json["transform"].is_null())
    {
        return;
    }

    auto json_transform = in_json["transform"];
        
    if(!json_transform["location"].is_null())
    {
        scene_object->location = ParseVec3(json_transform["location"]);
    }

    if(!json_transform["rotation"].is_null())
    {
        scene_object->rotation = ParseVec3(json_transform["rotation"]);
    }

    if(!json_transform["scale"].is_null())
    {
        scene_object->scale = ParseVec3(json_transform["scale"]);
    }
}

SRenderResourceDesc ParseRenderObjInfo(nlohmann::basic_json<> in_json)
{
    SRenderResourceDesc render_resource_desc;
    if(!in_json["shader_path"].is_null())
    {
        auto shader_json = in_json["shader_path"];
        if(!shader_json["vs"].is_null())
        {
            render_resource_desc.shader_paths.emplace(SRenderResourceDesc::EShaderType::vs,
                ToResourcePath(shader_json["vs"]));
        }

        if(!shader_json["fs"].is_null())
        {
            render_resource_desc.shader_paths.emplace(SRenderResourceDesc::EShaderType::fs,
                ToResourcePath(shader_json["fs"]));
        }
    }

    if(!in_json["model_path"].is_null())
    {
        render_resource_desc.model_path = ToResourcePath(in_json["model_path"]);
    }

    if(!in_json["texture_path"].is_null())
    {
        auto texture_json = in_json["texture_path"];
        if(!texture_json["diffuse"].is_null())
        {
            render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::diffuse,
                ToResourcePath(texture_json["diffuse"]));        
        }

        if(!texture_json["specular_map"].is_null())
        {
            render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::specular_map,
                ToResourcePath(texture_json["specular_map"]));   
        }
    }

    if(!in_json["color"].is_null())
    {
        render_resource_desc.color = ParseVec3(in_json["color"]);
    }
    
    return render_resource_desc;
}

bool CSceneLoader::LoadScene(const string& file_path, vector<shared_ptr<CRenderObj>>& render_objs,
    vector<shared_ptr<Light>>& lights)
{
    std::string yaml_content = Engine::ReadFile(ToResourcePath(file_path));
       
    json data = json::parse(yaml_content);
    auto scene = data["scene"];
    for(auto& child : scene)
    {
        if(child["object"] == "box")
        {
            SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(child);

            auto new_box = make_shared<CUtilityBox>(render_resource_desc);
            ParseTransform(child, new_box);
            
            render_objs.push_back(new_box);
        }
        else if(child["object"] == "model")
        {
            SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(child);
            
            auto new_model = make_shared<CModel>(render_resource_desc);
            ParseTransform(child, new_model);
            
            render_objs.push_back(new_model);
        }
        else if(child["object"] == "directional_light")
        {
            auto new_light = make_shared<DirectionalLight>();
            new_light->light_color = ParseVec3(child["light_color"]);
            ParseTransform(child, new_light);    
            
            lights.push_back(new_light);
        }
        else if(child["object"] == "point_light")
        {
            auto new_light = make_shared<PointLight>();
            new_light->light_color = ParseVec3(child["light_color"]);
            ParseTransform(child, new_light);

            lights.push_back(new_light);
        }
    }
    
    return true;
}
