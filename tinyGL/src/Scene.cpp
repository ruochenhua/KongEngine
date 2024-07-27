#include "Scene.h"
#include "Engine.h"
#include "light.h"
#include "model.h"
#include "utilityshape.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace tinyGL;

string CSceneLoader::ToResourcePath(const string& in_path)
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
                CSceneLoader::ToResourcePath(shader_json["vs"]));
        }

        if(!shader_json["fs"].is_null())
        {
            render_resource_desc.shader_paths.emplace(SRenderResourceDesc::EShaderType::fs,
                CSceneLoader::ToResourcePath(shader_json["fs"]));
        }
    }

    if(!in_json["model_path"].is_null())
    {
        render_resource_desc.model_path = CSceneLoader::ToResourcePath(in_json["model_path"]);
    }

    if(!in_json["texture_path"].is_null())
    {
        auto texture_json = in_json["texture_path"];
        if(!texture_json["diffuse"].is_null())
        {
            render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::diffuse,
                CSceneLoader::ToResourcePath(texture_json["diffuse"]));        
        }

        if(!texture_json["specular"].is_null())
        {
            render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::specular,
                CSceneLoader::ToResourcePath(texture_json["specular"]));   
        }
        
        if(!texture_json["normal"].is_null())
        {
            render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::normal,
                CSceneLoader::ToResourcePath(texture_json["normal"]));   
        }
    }

    if(!in_json["material"].is_null())
    {
        auto material_json = in_json["material"];
        if(!material_json["albedo"].is_null())
        {
            render_resource_desc.material.albedo = ParseVec3(material_json["albedo"]);    
        }

        if(!material_json["metallic"].is_null())
        {
            render_resource_desc.material.metallic = material_json["metallic"];    
        }

        if(!material_json["roughness"].is_null())
        {
            render_resource_desc.material.roughness = material_json["roughness"];    
        }

        if(!material_json["ao"].is_null())
        {
            render_resource_desc.material.ao = material_json["ao"];    
        }
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
