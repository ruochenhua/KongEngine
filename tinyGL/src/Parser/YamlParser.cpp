#include "YamlParser.h"

#include <yaml-cpp/yaml.h>

#include "Actor.h"
#include "Component/Mesh/ModelMeshComponent.h"
#include "Scene.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/BoxShape.h"
#include "Component/Mesh/SphereShape.h"
#include "glm/gtc/random.hpp"

using namespace tinyGL;

namespace YamlParser
{
    glm::vec3 ParseVec3(const vector<float>& in_vector)
    {
        assert(in_vector.size() == 3);
        return glm::vec3(in_vector[0], in_vector[1], in_vector[2]);
    }

    void ParseTransform(YAML::Node transform_node, shared_ptr<AActor> actor)
    {        
        if(transform_node["location"])
        {
            actor->location = ParseVec3(transform_node["location"].as<vector<float>>());
        }

        if(transform_node["rotation"])
        {
            actor->rotation = ParseVec3(transform_node["rotation"].as<vector<float>>());
        }

        if(transform_node["scale"])
        {
            actor->scale = ParseVec3(transform_node["scale"].as<vector<float>>());
        }

        if(transform_node["instancing"])
        {
            auto instancing = transform_node["instancing"];
            auto& instancing_info = actor->instancing_info;
            instancing_info.count = instancing["count"].as<int>();
            instancing_info.location_max = ParseVec3(instancing["location"]["max"].as<vector<float>>());
            instancing_info.location_min = ParseVec3(instancing["location"]["min"].as<vector<float>>());
            instancing_info.rotation_max = ParseVec3(instancing["rotation"]["max"].as<vector<float>>());
            instancing_info.rotation_min = ParseVec3(instancing["rotation"]["min"].as<vector<float>>());
            instancing_info.scale_max = ParseVec3(instancing["scale"]["max"].as<vector<float>>());
            instancing_info.scale_min = ParseVec3(instancing["scale"]["min"].as<vector<float>>());
        
            // actor->GenInstanceModelMatrix();
        }
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

            if(texture_node["metallic"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::metallic,
                    CSceneLoader::ToResourcePath(texture_node["metallic"].as<string>()));   
            }

            if(texture_node["roughness"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::roughness,
                    CSceneLoader::ToResourcePath(texture_node["roughness"].as<string>()));   
            }

            if(texture_node["ao"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::ambient_occlusion,
                    CSceneLoader::ToResourcePath(texture_node["ao"].as<string>()));   
            }

            if(texture_node["glow"])
            {
                render_resource_desc.texture_paths.emplace(SRenderResourceDesc::ETextureType::glow,
                    CSceneLoader::ToResourcePath(texture_node["glow"].as<string>()));   
            }
        }

        if(in_node["material"])
        {
            render_resource_desc.bOverloadMaterial = true;
            auto material_node = in_node["material"];
            if(material_node["albedo"])
            {
                render_resource_desc.material.albedo = glm::vec4(ParseVec3(material_node["albedo"].as<vector<float>>()), 1.0);    
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

    void ParseComponent(YAML::Node actor, shared_ptr<AActor> new_actor)
    {        
        auto actor_components = actor["component"];
        for(auto component : actor_components)
        {
            string component_type = component["type"].as<string>();
            if(component_type == "box")
            {
                SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                auto mesh_comp = make_shared<CBoxShape>(render_resource_desc);

                new_actor->AddComponent(mesh_comp);
            }
            else if(component_type == "sphere")
            {
                SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                auto mesh_comp = make_shared<SphereShape>(render_resource_desc);

                new_actor->AddComponent(mesh_comp);
            }
            else if(component_type == "mesh")
            {
                SRenderResourceDesc render_resource_desc = ParseRenderObjInfo(component);
                auto mesh_comp = make_shared<CModelMeshComponent>(render_resource_desc);

                new_actor->AddComponent(mesh_comp);   
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

        if(actor["transform"])
        {
            ParseTransform(actor["transform"], new_actor);
        }

        ParseComponent(actor, new_actor);
        scene_actors.push_back(new_actor);
    }
}