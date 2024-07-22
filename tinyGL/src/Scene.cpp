#include "Scene.h"

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "ryml_all.hpp"
#include "utilityshape.h"
using namespace tinyGL;

vector<SRenderInfo> SceneLoader::LoadScene(const string& file_path)
{
    std::string yaml_content;
    std::ifstream yaml_stream(file_path, std::ios::in);
    if (yaml_stream.is_open()) {
        std::stringstream sstr;
        sstr << yaml_stream.rdbuf();
        yaml_content = sstr.str();
        yaml_stream.close();
    }

    vector<SRenderInfo> render_infos;
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
                string vs_path_string(vs_path.data(), vs_path.len);
                string fs_path_string(fs_path.data(), fs_path.len);

                CUtilityBox* test_box = new CUtilityBox({RESOURCE_PATH+vs_path_string, RESOURCE_PATH+fs_path_string});
                render_infos.push_back(test_box->GetRenderInfo());
            }
        }
    }

    return render_infos;
}
