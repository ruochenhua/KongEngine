#include "YamlParser.h"

#include <yaml-cpp/yaml.h>

#include "Actor.h"
#include "render.h"
#include "Component/Mesh/ModelMeshComponent.h"
#include "Scene.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/BoxShape.h"
#include "Component/Mesh/GerstnerWaveWater.h"
#include "Component/Mesh/QuadShape.h"
#include "Component/Mesh/SphereShape.h"
#include "Component/Mesh/Terrain.h"
#include "Component/Mesh/Water.h"
#include "glm/gtc/random.hpp"

using namespace Kong;

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

    void ParseSpawnerTransform(YAML::Node transform_node, shared_ptr<AActor> actor)
    {
        if(transform_node["location"])
        {
            glm::vec3 location_min = ParseVec3(transform_node["location"]["min"].as<vector<float>>());
            glm::vec3 location_max = ParseVec3(transform_node["location"]["max"].as<vector<float>>());
            actor->location = glm::linearRand(location_min, location_max);
        }

        if(transform_node["rotation"])
        {
            glm::vec3 rotation_min = ParseVec3(transform_node["rotation"]["min"].as<vector<float>>());
            glm::vec3 rotation_max = ParseVec3(transform_node["rotation"]["max"].as<vector<float>>());
            actor->rotation = glm::linearRand(rotation_min, rotation_max);
        }

        if(transform_node["scale"])
        {
            glm::vec3 scale_min = ParseVec3(transform_node["scale"]["min"].as<vector<float>>());
            glm::vec3 scale_max = ParseVec3(transform_node["scale"]["max"].as<vector<float>>());
            actor->scale = glm::linearRand(scale_min, scale_max);
        }
    }

    void ParseMeshMaterial(YAML::Node mesh_node, shared_ptr<CMeshComponent> mesh_component)
    {
        // 创建shader
        if(mesh_node["shader_type"])
        {
            mesh_component->shader_data = ShaderManager::GetShader(mesh_node["shader_type"].as<string>());
        }
        else if(mesh_node["shader_path"])
        {
            map<EShaderType, string> shader_cache;
            auto shader_node = mesh_node["shader_path"];
            if(shader_node["vs"])
            {
                shader_cache.emplace(EShaderType::vs,
                    CSceneLoader::ToResourcePath(shader_node["vs"].as<string>()));
            }

            if(shader_node["fs"])
            {
                shader_cache.emplace(EShaderType::fs,
                    CSceneLoader::ToResourcePath(shader_node["fs"].as<string>()));
            }

            mesh_component->shader_data = make_shared<Shader>(shader_cache);
        }

        if(mesh_node["material"])
        {
            auto material_node = mesh_node["material"];
            // 读取材质信息
            SMaterial tmp_material;
            if(material_node["diffuse"])
            {
                if(material_node["diffuse"].IsSequence())
                {
                    tmp_material.albedo = glm::vec4(ParseVec3(material_node["diffuse"].as<vector<float>>()), 1.0);    
                }
                else
                {
                    tmp_material.diffuse_tex_id =
                        ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(material_node["diffuse"].as<string>()));
                }
            }

            if(material_node["metallic"])
            {
                try
                {
                    tmp_material.metallic = material_node["metallic"].as<float>();
                }
                catch (const YAML::BadConversion& e)
                {
                    tmp_material.metallic_tex_id =
                        ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(material_node["metallic"].as<string>()));
                }
            }

            if(material_node["roughness"])
            {
                try
                {
                    tmp_material.roughness = material_node["roughness"].as<float>();    
                }
                catch (const YAML::BadConversion& e)
                {
                    tmp_material.roughness_tex_id =
                        ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(material_node["roughness"].as<string>()));
                }    
            }

            if(material_node["ao"])
            {
                try
                {
                    tmp_material.ao = material_node["ao"].as<float>();    
                }
                catch (const YAML::BadConversion& e)
                {
                    tmp_material.ao_tex_id =
                        ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(material_node["ao"].as<string>()));
                }    
            }
            
            if(material_node["normal"])
            {
                tmp_material.normal_tex_id =
                    ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(material_node["normal"].as<string>()));
            }

            mesh_component->override_render_info.material = tmp_material;
            mesh_component->use_override_material = true;
        }
    }

    void ParseComponent(YAML::Node actor, shared_ptr<AActor> new_actor)
    {        
        auto actor_components = actor["component"];
        for(auto component : actor_components)
        {
            string component_type = component["type"].as<string>();
            if(component_type == "box")
            {
                auto mesh_comp = make_shared<CBoxShape>();
                ParseMeshMaterial(component, mesh_comp);
                new_actor->AddComponent(mesh_comp);
            }
            else if(component_type == "sphere")
            {
                auto mesh_comp = make_shared<SphereShape>();
                ParseMeshMaterial(component, mesh_comp);
                new_actor->AddComponent(mesh_comp);
            }
            else if(component_type == "quad")
            {
                auto mesh_comp = make_shared<CQuadShape>();
                ParseMeshMaterial(component, mesh_comp);
                new_actor->AddComponent(mesh_comp);
            }
            else if(component_type == "mesh")
            {
                string model_path = component["model_path"].as<string>();
                auto mesh_comp = make_shared<CModelMeshComponent>(CSceneLoader::ToResourcePath(model_path));
                ParseMeshMaterial(component, mesh_comp);
                
                new_actor->AddComponent(mesh_comp);   
            }
            else if(component_type == "terrain")
            {
                shared_ptr<Terrain> terrain_comp;
                if(component["height_map_path"])
                {
                    string height_map_path = component["height_map_path"].as<string>();
                    terrain_comp = make_shared<Terrain>(CSceneLoader::ToResourcePath(height_map_path));
                }
                else
                {
                    terrain_comp = make_shared<Terrain>();
                }
                
                if (component["size"])
                {
                    terrain_comp->terrain_size = component["size"].as<int>();    
                }

                if (component["resolution"])
                {
                    terrain_comp->terrain_res = component["resolution"].as<int>();
                }

                if (component["height_scale"])
                {
                    terrain_comp->height_scale_ = component["height_scale"].as<float>();
                }

                if (component["height_shift"])
                {
                    terrain_comp->height_shift_ = component["height_shift"].as<float>();
                }
                
                new_actor->AddComponent(terrain_comp);
            }
            // 普通的水, simple water
            else if (component_type == "water")
            {
                shared_ptr<Water> water_comp = make_shared<Water>();
                if (component["dudv_map_path"])
                {
                    water_comp->LoadDudvMapTexture(component["dudv_map_path"].as<string>());
                }

                if (component["normal_map_path"])
                {
                    water_comp->LoadNormalTexture(component["normal_map_path"].as<string>());
                }
                
                new_actor->AddComponent(water_comp);

                auto& render_sys = KongRenderModule::GetRenderModule();
                render_sys.SetRenderWater(new_actor);
            }
            // 带有gerstner wave模拟的水
            else if (component_type == "gerstner_wave_water")
            {
                shared_ptr<GerstnerWaveWater> water_comp = make_shared<GerstnerWaveWater>();

                if (component["size"])
                {
                    water_comp->water_size = component["size"].as<int>();
                }

                if (component["resolution"])
                {
                    water_comp->water_resolution = component["resolution"].as<int>();
                }

                if (component["height_scale"])
                {
                    water_comp->height_scale_ = component["height_scale"].as<float>();
                }

                if (component["height_shift"])
                {
                    water_comp->height_shift_ = component["height_shift"].as<float>();
                    new_actor->location.y = water_comp->height_shift_;  // 需要记录shift来决定reflection相机的位置
                }

                if (component["dudv_map_path"])
                {
                    water_comp->LoadDudvMapTexture(component["dudv_map_path"].as<string>());
                }

                if (component["normal_map_path"])
                {
                    water_comp->LoadNormalTexture(component["normal_map_path"].as<string>());
                }
                
                new_actor->AddComponent(water_comp);
                auto& render_sys = KongRenderModule::GetRenderModule();
                render_sys.SetRenderWater(new_actor);
            }
            else if(component_type == "directional_light")
            {
                auto dirlight_comp = make_shared<CDirectionalLightComponent>();
                if(component["light_color"])
                {
                    dirlight_comp->light_color = ParseVec3(component["light_color"].as<vector<float>>());
                }
                else
                {
                    // 不填光的颜色就随机给一个
                    dirlight_comp->light_color = glm::abs(glm::ballRand(1.f));    
                }

                if(component["light_intensity"])
                {
                    // 填了光的强度的话，需要乘上去
                    dirlight_comp->light_intensity = component["light_intensity"].as<float>(); 
                    dirlight_comp->light_color *= dirlight_comp->light_intensity;
                }

                // 平行光默认打开阴影贴图
                if(component["make_shadow"])
                {
                    dirlight_comp->TurnOnShadowMap(component["make_shadow"].as<bool>());
                }
                else
                {
                    dirlight_comp->TurnOnShadowMap(true);
                }

                if(component["reflective_shadow_map"])
                {
                    dirlight_comp->TurnOnReflectiveShadowMap(component["reflective_shadow_map"].as<bool>());    
                }
                
                new_actor->AddComponent(dirlight_comp);
            }
            else if(component_type == "point_light")
            {
                auto pointlight_comp = make_shared<CPointLightComponent>();
                if(component["light_color"])
                {
                    pointlight_comp->light_color = ParseVec3(component["light_color"].as<vector<float>>());
                }
                else
                {
                    // 不填光的颜色就随机给一个
                    pointlight_comp->light_color = glm::abs(glm::ballRand(1.f));    
                }

                if(component["light_intensity"])
                {
                    // 填了光的强度的话，需要乘上去
                    pointlight_comp->light_color *= component["light_intensity"].as<float>();
                }

                // 点光源默认不开启阴影贴图
                if(component["make_shadow"])
                {
                    pointlight_comp->TurnOnShadowMap(component["make_shadow"].as<bool>());
                }
                
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
        // 是actor spawner，会需要生成多个actor
        if(actor["type"] && actor["type"].as<string>() == "spawner")
        {
            int count = actor["count"].as<int>();
            for(int i = 0; i < count; i++)
            {
                auto new_actor = make_shared<AActor>();
                new_actor->name = actor["actor"].as<string>();
                if(actor["transform"])
                {
                    ParseSpawnerTransform(actor["transform"], new_actor);
                }
                
                ParseComponent(actor, new_actor);
                scene_actors.push_back(new_actor);
            }
        }
        else
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

    if(scene_node["setting"])
    {
        auto& render_sys = KongRenderModule::GetRenderModule();
        auto setting = scene_node["setting"];
        if(setting["skybox"])
        {
            auto skybox_node = setting["skybox"];
            if(skybox_node["render_sky_env_status"])
            {
                auto render_sky_env_status = skybox_node["render_sky_env_status"].as<int>();
                render_sys.render_sky_env_status = render_sky_env_status;
            }

            if(skybox_node["render_cloud"])
            {
                auto render_cloud = skybox_node["render_cloud"].as<bool>();
                render_sys.m_SkyBox.render_cloud = render_cloud;
            }
        }
    }
}