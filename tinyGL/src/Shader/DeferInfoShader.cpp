#include "DeferInfoShader.h"

using namespace Kong;
void DeferInfoShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/DeferInfo.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/DeferInfo.frag")}
    };

    shader_id = LoadShaders(shader_path_map);

    // 设定各个贴图资源
    Use();
    SetInt("diffuse_texture", DIFFUSE_TEX_SHADER_ID);
    SetInt("normal_texture", NORMAL_TEX_SHADER_ID);
    SetInt("roughness_texture", ROUGHNESS_TEX_SHADER_ID);
    SetInt("metallic_texture", METALLIC_TEX_SHADER_ID);
    SetInt("ao_texture", AO_TEX_SHADER_ID);
    SetInt("skybox_texture", SKYBOX_TEX_SHADER_ID);
    SetInt("skybox_diffuse_irradiance_texture", SKYBOX_DIFFUSE_IRRADIANCE_TEX_SHADER_ID);
    SetInt("skybox_prefilter_texture", SKYBOX_PREFILTER_TEX_SHADER_ID);
    SetInt("skybox_brdf_lut_texture", SKYBOX_BRDF_LUT_TEX_SHADER_ID);
    // shadow map应该不需要，不涉及光照部分
    // SetInt("shadow_map", DIRLIGHT_SM_TEX_SHADER_ID);
    // for(unsigned int i = 0; i < 4; ++i)
    // {
    //     stringstream ss;
    //     ss << "shadow_map_pointlight[" << i << "]";
    //     SetInt(ss.str(), POINTLIGHT_SM_TEX_SHADER_ID + i);
    // }
}
