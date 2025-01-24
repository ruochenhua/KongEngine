#include "SkyboxShader.h"

#include "Scene.h"

using namespace Kong;

SkyboxShader::SkyboxShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/skybox.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/skybox.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path_map);
    assert(shader_id, "load skybox shader failed");
}

AtmosphereShader::AtmosphereShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/atmosphere.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/atmosphere.frag")},
    };

    shader_id = Shader::LoadShaders(shader_path_map);
    assert(shader_id, "load atmosphere shader failed");
    
    SetInt("depth_map", 0);
    SetInt("cloud", 1);
    SetInt("worley32", 2);
    SetInt("weatherTex", 3);
}

EquirectangularToCubemapShader::EquirectangularToCubemapShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/sphere_to_cube.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path_map);
    assert(shader_id, "load skybox shader failed");
}

IrradianceCalculationShader::IrradianceCalculationShader()
{
    // 辐照度预计算shader
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/irradiance_map.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path_map);
    assert(shader_id, "load skybox shader failed");
}

PrefilterCalculationShader::PrefilterCalculationShader()
{
    // 预滤波环境贴图计算shader
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/prefilter_map.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path_map);
    assert(shader_id, "load skybox shader failed");
}

BRDFLutCalculationShader::BRDFLutCalculationShader()
{
    // brdf预计算贴图shader
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/brdf_lut.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/brdf_lut.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path_map);
    assert(shader_id, "load skybox shader failed");
}
