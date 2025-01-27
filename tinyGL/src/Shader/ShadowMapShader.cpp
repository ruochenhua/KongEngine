#include "ShadowMapShader.h"

#include "Scene.hpp"

using namespace Kong;
using namespace glm;

DirectionalLightShadowMapShader::DirectionalLightShadowMapShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap.frag")}
    };
    
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");
}

DirectionalLightCSMShader::DirectionalLightCSMShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/shadow/csm.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/shadow/csm.frag")},
        {gs, CSceneLoader::ToResourcePath("shader/shadow/csm.geom")}
    };
    
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
}

PointLightShadowMapShader::PointLightShadowMapShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_pointlight.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_pointlight.frag")},
        {gs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_pointlight.geom")}
    };
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");

}
