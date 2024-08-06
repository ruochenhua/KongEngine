#include "YamlParser.h"

#include <yaml-cpp/yaml.h>

#include "Actor.h"
#include "model.h"
#include "Scene.h"
#include "utilityshape.h"

using namespace tinyGL;

namespace YamlParser
{
    glm::vec3 ParseVec3(const vector<float>& in_vector)
    {
        assert(in_vector.size() == 3);
        return glm::vec3(in_vector[0], in_vector[1], in_vector[2]);
    }


    void ParseTransform(YAML::Node transform_node, shared_ptr<CTransformComponent> transform_comp)
    {        
        if(transform_node["location"])
        {
            transform_comp->location = ParseVec3(transform_node["location"].as<vector<float>>());
        }

        if(transform_node["rotation"])
        {
            transform_comp->rotation = ParseVec3(transform_node["rotation"].as<vector<float>>());
        }

        if(transform_node["scale"])
        {
            transform_comp->scale = ParseVec3(transform_node["scale"].as<vector<float>>());
        }

        if(transform_node["instancing"])
        {
            auto instancing = transform_node["instancing"];
            auto& instancing_info = transform_comp->instancing_info;
            instancing_info.count = instancing["count"].as<int>();
            instancing_info.location_max = ParseVec3(instancing["location"]["max"].as<vector<float>>());
            instancing_info.location_min = ParseVec3(instancing["location"]["min"].as<vector<float>>());
            instancing_info.rotation_max = ParseVec3(instancing["rotation"]["max"].as<vector<float>>());
            instancing_info.rotation_min = ParseVec3(instancing["rotation"]["min"].as<vector<float>>());
            instancing_info.scale_max = ParseVec3(instancing["scale"]["max"].as<vector<float>>());
            instancing_info.scale_min = ParseVec3(instancing["scale"]["min"].as<vector<float>>());

            // transform_comp->GenInstanceModelMatrix();
        }
    }

    void ParseInstancing(YAML::Node in_node, shared_ptr<CTransformComponent> scene_object)
    {
        // auto& instancing_info = scene_object->instancing_info;
        //
        // auto instancing = in_json["instancing"];
        // instancing_info.count = instancing["count"];
        //
        // if(!instancing["location"].is_null())
        // {
        //     instancing_info.location_min = ParseVec3(instancing["location"]["min"]);
        //     instancing_info.location_max = ParseVec3(instancing["location"]["max"]);
        // }
        //
        // if(!instancing["rotation"].is_null())
        // {
        //     instancing_info.rotation_min = ParseVec3(instancing["rotation"]["min"]);
        //     instancing_info.rotation_max = ParseVec3(instancing["rotation"]["max"]);
        // }
        //
        // if(!instancing["scale"].is_null())
        // {
        //     instancing_info.scale_min = ParseVec3(instancing["scale"]["min"]);
        //     instancing_info.scale_max = ParseVec3(instancing["scale"]["max"]);
        // }
    }

    SRenderResourceDesc ParseRenderObjInfo(YAML::Node in_node)
    {
        SRenderResourceDesc render_resource_desc;
        if(in_node["shader_type"])
        {
            render_resource_desc.shader_type = in_node["shader_type"].as<string>();
        }
        else if(in_node["shader_path"])
        {
            auto shader_node = in_node["shader_path"];
            if(shader_node["vs"])
            {
                render_resource_desc.shader_paths.emplace(EShaderType::vs,
                    CSceneLoader::ToResourcePath(shader_node["vs"].as<string>()));
            }

            if(shader_node["fs"])
            {
                render_resource_desc.shader_paths.emplace(EShaderType::fs,
                    CSceneLoader::ToResourcePath(shader_node["fs"].as<string>()));
            }
        }

        if(in_node["model_path"])
        {
            render_resource_desc.model_path = CSceneLoader::ToResourcePath(in_node["model_path"].as<string>());
        }

        if(in_node["texture_path"])
        {
            auto texture_node = in_node["texture_path"];
            if(texture_node["diffuse"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::diffuse,
                    CSceneLoader::ToResourcePath(texture_node["diffuse"].as<string>()));        
            }

            if(texture_node["specular"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::specular,
                    CSceneLoader::ToResourcePath(texture_node["specular"].as<string>()));   
            }
        
            if(texture_node["normal"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::normal,
                    CSceneLoader::ToResourcePath(texture_node["normal"].as<string>()));   
            }
        
            if(texture_node["tangent"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::tangent,
                    CSceneLoader::ToResourcePath(texture_node["tangent"].as<string>()));   
            }
        }

        if(in_node["material"])
        {
            auto material_node = in_node["material"];
            if(material_node["albedo"])
            {
                render_resource_desc.material.albedo = ParseVec3(material_node["albedo"].as<vector<float>>());    
            }

            if(material_node["metallic"])
            {
                render_resource_desc.material.metallic = material_node["metallic"].as<float>();    
            }

            if(material_node["roughness"])
            {
                render_resource_desc.material.roughness = material_node["roughness"].as<float>();    
            }

            if(material_node["ao"])
            {
                render_resource_desc.material.ao = material_node["ao"].as<float>();    
            }
        }
    
        return render_resource_desc;
    }

}
using namespace YamlParser;
void CYamlParser::ParseYamlFile(const std::string& scene_content, std::vector<std::shared_ptr<AActor>>& scene_actors)
{
    YAML::Node scene_node = YAML::Load(scene_content);

    auto scene = scene_node["scene"];
    for(auto& actor : scene)
    {
        auto new_actor = make_shared<AActor>();
        new_actor->name = actor["actor"].as<string>();

        auto actor_components = actor["component"];
        for(auto component : actor_components)
        {
            string component_type = component["type"].as<string>();
            if(component_type == "box")
            {
                SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                auto mesh_comp = make_shared<CUtilityBox>(render_resource_desc);

                new_actor->AddComponent(mesh_comp);
            }
            else if(component_type == "mesh")
            {
                SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                auto mesh_comp = make_shared<CModelMeshComponent>(render_resource_desc);

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
                dirlight_comp->light_color = ParseVec3(component["light_color"].as<vector<float>>());
                // ParseTransform(actor, new_light);    
        
                //lights.push_back(new_light);
                new_actor->AddComponent(dirlight_comp);
            }
            else if(component_type == "point_light")
            {
                auto pointlight_comp = make_shared<CPointLightComponent>();
                pointlight_comp->light_color = ParseVec3(component["light_color"].as<vector<float>>());
                //ParseTransform(actor, new_light);
    
                new_actor->AddComponent(pointlight_comp);
            }
        }

        scene_actors.push_back(new_actor);
    }
}