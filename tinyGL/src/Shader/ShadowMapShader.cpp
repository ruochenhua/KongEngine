#include "ShadowMapShader.h"

#include "Scene.h"

using namespace tinyGL;
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

void PointLightShadowMapShader::UpdateShadowMapRender(
    const glm::vec3& location,
    const glm::mat4& model_mat,
    const glm::vec2& near_far_plane)
{
    // 点光源的阴影贴图
    GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
    float near_plane = near_far_plane.x;
    float far_plane = near_far_plane.y;
    mat4 shadow_proj = perspective(radians(90.f), aspect, near_plane, far_plane);
    
    // 方向可以固定是朝向六个方向
    vector<mat4> shadow_transforms;
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(1,0,0), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(-1,0,0), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,1,0), vec3(0,0,1)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,-1,0), vec3(0,0,-1)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,0,1), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(location, location+vec3(0,0,-1), vec3(0,-1,0)));

    Use();
    SetFloat("far_plane", far_plane);
    for(int i = 0; i < 6; ++i)
    {
        stringstream shadow_matrices_stream;
        shadow_matrices_stream <<  "shadow_matrices[" << i << "]";
        SetMat4(shadow_matrices_stream.str(), shadow_transforms[i]);
    }
    SetVec3("light_pos", location);
    // RenderScene();
    SetMat4("model", model_mat);
}
