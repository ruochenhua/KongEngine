#include "JsonParser.h"

#include <nlohmann/json.hpp>

#include "Actor.h"
#include "ModelMeshComponent.h"
#include "Scene.h"
#include "BoxShape.h"
#include "QuadShape.h"
using json = nlohmann::json;

using namespace tinyGL;

namespace JsonParser
{
    glm::vec3 ParseVec3(const vector<float>& in_vector)
    {
        assert(in_vector.size() == 3);
        return glm::vec3(in_vector[0], in_vector[1], in_vector[2]);
    }


    void ParseTransform(nlohmann::basic_json<> json_transform, shared_ptr<CTransformComponent> transform_comp)
    {        
        if(!json_transform["location"].is_null())
        {
            transform_comp->location = ParseVec3(json_transform["location"]);
        }

        if(!json_transform["rotation"].is_null())
        {
            transform_comp->rotation = ParseVec3(json_transform["rotation"]);
        }

        if(!json_transform["scale"].is_null())
        {
            transform_comp->scale = ParseVec3(json_transform["scale"]);
        }

        if(!json_transform["instancing"].is_null())
        {
            auto instancing = json_transform["instancing"];
            auto& instancing_info = transform_comp->instancing_info;
            instancing_info.count = instancing["count"];
            instancing_info.location_max = ParseVec3(instancing["location"]["max"]);
            instancing_info.location_min = ParseVec3(instancing["location"]["min"]);
            instancing_info.rotation_max = ParseVec3(instancing["rotation"]["max"]);
            instancing_info.rotation_min = ParseVec3(instancing["rotation"]["min"]);
            instancing_info.scale_max = ParseVec3(instancing["scale"]["max"]);
            instancing_info.scale_min = ParseVec3(instancing["scale"]["min"]);

            transform_comp->GenInstanceModelMatrix();
        }
    }

    SRenderResourceDesc ParseRenderObjInfo(nlohmann::basic_json<> in_json)
    {
        SRenderResourceDesc render_resource_desc;
        if(!in_json["shader_type"].is_null())
        {
            render_resource_desc.shader_type = in_json["shader_type"];
        }
        else if(!in_json["shader_path"].is_null())
        {
            auto shader_json = in_json["shader_path"];
            if(!shader_json["vs"].is_null())
            {
                render_resource_desc.shader_paths.emplace(EShaderType::vs,
                    CSceneLoader::ToResourcePath(shader_json["vs"]));
            }

            if(!shader_json["fs"].is_null())
            {
                render_resource_desc.shader_paths.emplace(EShaderType::fs,
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
        
            if(!texture_json["tangent"].is_null())
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::tangent,
                    CSceneLoader::ToResourcePath(texture_json["tangent"]));   
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
}

using namespace JsonParser;
    void CJsonParser::ParseJsonFile(const std::string& scene_content, vector<shared_ptr<AActor>>& scene_actors)
    {
        // 格式为json，读取json文件
        json data = json::parse(scene_content);
        
        auto scene = data["scene"];
        for(auto& actor : scene)
        {
            auto new_actor = make_shared<AActor>();
            new_actor->name = actor["actor"];

            auto actor_components = actor["component"];
            for(auto component : actor_components)
            {
                string component_type = component["type"];
                if(component_type == "box")
                {
                    SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                    auto mesh_comp = make_shared<CBoxShape>(render_resource_desc);

                    new_actor->AddComponent(mesh_comp);
                }
                else if(component_type == "mesh")
                {
                    SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                    auto mesh_comp = make_shared<CModelMeshComponent>(render_resource_desc);

                    new_actor->AddComponent(mesh_comp);   
                }
                else if(component_type == "quad")
                {
                    SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                    auto mesh_comp = make_shared<CQuadShape>(render_resource_desc);

                    new_actor->AddComponent(mesh_comp);
                }
                else if(component_type == "transform")
                {
                    auto trans_comp = make_shared<CTransformComponent>();
                    ParseTransform(component, trans_comp);
                
                    new_actor->AddComponent(trans_comp);
                }
                else if(component_type == "directional_light")
                {
                    auto dirlight_comp = make_shared<CDirectionalLightComponent>();
                    dirlight_comp->light_color = ParseVec3(component["light_color"]);

                    new_actor->AddComponent(dirlight_comp);
                }
                else if(component_type == "point_light")
                {
                    auto pointlight_comp = make_shared<CPointLightComponent>();
                    pointlight_comp->light_color = ParseVec3(component["light_color"]);
        
                    new_actor->AddComponent(pointlight_comp);
                }
                
            }

            scene_actors.push_back(new_actor);
        }
    }