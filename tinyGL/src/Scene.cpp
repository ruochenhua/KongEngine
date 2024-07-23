#include "Scene.h"

#include "utilityshape.h"

#include <nlohmann/json.hpp>

#include "light.h"
#include "model.h"
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


bool CSceneLoader::LoadScene(const string& file_path, vector<shared_ptr<CRenderObj>>& render_objs,
    vector<shared_ptr<Light>>& lights)
{
    std::string yaml_content;
    std::ifstream yaml_stream(ToResourcePath(file_path), std::ios::in);
    if (yaml_stream.is_open()) {
        std::stringstream sstr;
        sstr << yaml_stream.rdbuf();
        yaml_content = sstr.str();
        yaml_stream.close();
    }
    
    json data = json::parse(yaml_content);
    auto scene = data["scene"];
    for(auto& child : scene)
    {
        if(child["object"] == "box")
        {
            string vs_path_string = ToResourcePath(child["shader_path"]["vs"]);
            string fs_path_string = ToResourcePath(child["shader_path"]["fs"]);

            vector<string> shader_path = {vs_path_string, fs_path_string};
            auto new_box = make_shared<CUtilityBox>(shader_path);
            
            ParseTransform(child, new_box);
            
            render_objs.push_back(new_box);
        }
        else if(child["object"] == "model")
        {
            string vs_path_string = ToResourcePath(child["shader_path"]["vs"]);
            string fs_path_string = ToResourcePath(child["shader_path"]["fs"]);

            vector<string> shader_path = {vs_path_string, fs_path_string};
            string model_path = ToResourcePath(child["model_path"]);
            string diffuse_tex_path = ToResourcePath(child["diffuse_tex_path"]);
            auto new_model = make_shared<CModel>(model_path, diffuse_tex_path, shader_path);
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
