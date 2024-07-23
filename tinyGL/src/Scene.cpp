#include "Scene.h"

#include "utilityshape.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace tinyGL;

string ToResourcePath(const string& in_path)
{
    return RESOURCE_PATH + in_path;
}

string GetResourcePathFromSubstr(const string& in_str)
{
    return ToResourcePath(in_str);
}

glm::vec3 ParseVec3(const vector<float>& in_vector)
{
    assert(in_vector.size() == 3);
    return glm::vec3(in_vector[0], in_vector[1], in_vector[2]);
}

vector<shared_ptr<CRenderObj>> CSceneLoader::LoadScene(const string& file_path)
{
    std::string yaml_content;
    std::ifstream yaml_stream(ToResourcePath(file_path), std::ios::in);
    if (yaml_stream.is_open()) {
        std::stringstream sstr;
        sstr << yaml_stream.rdbuf();
        yaml_content = sstr.str();
        yaml_stream.close();
    }

    vector<shared_ptr<CRenderObj>> render_infos;
    json data = json::parse(yaml_content);
    auto scene = data["scene"];
    for(auto& child : scene)
    {
        if(child["object"] == "box")
        {
            string vs_path = child["shader_path"]["vs"];
            string fs_path = child["shader_path"]["fs"];
            string vs_path_string = GetResourcePathFromSubstr(vs_path);
            string fs_path_string = GetResourcePathFromSubstr(fs_path);

            vector<string> shader_path = {vs_path_string, fs_path_string};
            auto test_box = make_shared<CUtilityBox>(shader_path);
            
            if(!child["transform"].is_null())
            {
                vector<float> object_location = child["transform"]["location"];
                test_box->location = ParseVec3(object_location);
                
                vector<float> object_rotation = child["transform"]["rotation"];
                test_box->rotation = ParseVec3(object_rotation);
                
                test_box->scale = ParseVec3(child["transform"]["scale"]);
            }
            
            render_infos.push_back(test_box);
        }
        else if(child["object"] == "model")
        {
            
        }
    }

    
    return render_infos;
}

