#include "SkyboxShader.h"

using namespace Kong;

SkyboxShader::SkyboxShader()
{
    map<EShaderType, string> shader_path = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/skybox.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/skybox.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path);
    assert(shader_id, "load skybox shader failed");
}

EquirectangularToCubemapShader::EquirectangularToCubemapShader()
{
    map<EShaderType, string> shader_path = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/sphere_to_cube.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path);
    assert(shader_id, "load skybox shader failed");
}

IrradianceCalculationShader::IrradianceCalculationShader()
{
    // 辐照度预计算shader
    map<EShaderType, string> shader_path = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/irradiance_map.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path);
    assert(shader_id, "load skybox shader failed");
}

PrefilterCalculationShader::PrefilterCalculationShader()
{
    // 预滤波环境贴图计算shader
    map<EShaderType, string> shader_path = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/prefilter_map.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path);
    assert(shader_id, "load skybox shader failed");
}

BRDFLutCalculationShader::BRDFLutCalculationShader()
{
    // brdf预计算贴图shader
    map<EShaderType, string> shader_path = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/brdf_lut.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/brdf_lut.frag")}
    };
	
    shader_id = Shader::LoadShaders(shader_path);
    assert(shader_id, "load skybox shader failed");
}
