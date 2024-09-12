#include "ShadowMapShader.h"

#include "Scene.h"

using namespace Kong;
using namespace glm;

void DirectionalLightShadowMapShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap.frag")}
    };
    
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");
}

void DirectionalLightCSMShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/csm_shadowmap.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/csm_shadowmap.frag")},
        {EShaderType::gs, CSceneLoader::ToResourcePath("shader/csm_shadowmap.geom")}
    };
    
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
}

void PointLightShadowMapShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap_pointlight.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap_pointlight.frag")},
        {EShaderType::gs, CSceneLoader::ToResourcePath("shader/shadowmap_pointlight.geom")}
    };
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");

}