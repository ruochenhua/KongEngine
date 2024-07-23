#include "Scene.h"

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "ryml_all.hpp"
#include "utilityshape.h"
using namespace tinyGL;

string ToResourcePath(const string& in_path)
{
    return RESOURCE_PATH + in_path;
}

string GetResourcePathFromSubstr(const c4::csubstr& in_substr)
{
    string std_substr(in_substr.data(), in_substr.len);
    return ToResourcePath(std_substr);
}

// glm::vec3 ParseVec3(c4::yml::NodeRef in_node)
// {
//     float value = ryml::fmt::real(in_node[0], 6, ryml::FTOA_FLOAT);
//     return glm::vec3(value, value, value);
// }

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
    ryml::Tree tree = ryml::parse_in_place(ryml::to_substr(yaml_content));
    auto scene = tree["scene"];
    
    if(scene.is_seq())
    {
        for(auto child : scene.children())
        {
            if(child["object"].val() == "box")
            {
                auto vs_path = child["shader_path"]["vs"].val();
                auto fs_path = child["shader_path"]["fs"].val();
                string vs_path_string = GetResourcePathFromSubstr(vs_path);
                string fs_path_string = GetResourcePathFromSubstr(fs_path);

                vector<string> shader_path = {vs_path_string, fs_path_string};
                auto test_box = make_shared<CUtilityBox>(shader_path);
                
                if(child["transform"].has_key())
                {
                    // auto object_location = child["transform"]["location"];
                    // test_box->location = ParseVec3(object_location);
                    //
                    // auto object_rotation = child["transform"]["rotation"];
                    // test_box->rotation = ParseVec3(object_rotation);
                    //
                    // test_box->scale = ParseVec3(child["transform"]["scale"]);
                }
                
                render_infos.push_back(test_box);
            }
            else if(child["object"].val() == "model")
            {
                
            }
        }
    }

    return render_infos;
}

