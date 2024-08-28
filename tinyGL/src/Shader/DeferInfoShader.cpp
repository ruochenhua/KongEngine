#include "DeferInfoShader.h"

#include "render.h"
#include "Component/LightComponent.h"

using namespace Kong;
void DeferInfoShader::InitDefaultShader()
{
    shader_path_map = {
        {EShaderType::vs, CSceneLoader::ToResourcePath("shader/defer_info.vert")},
        {EShaderType::fs, CSceneLoader::ToResourcePath("shader/defer_info.frag")}
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

void DeferredBRDFShader::InitDefaultShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/defer_pbr.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/defer_pbr.frag")},
    };
    shader_id = Shader::LoadShaders(shader_path_map);
    
    assert(shader_id, "Shader load failed!");
    
    Use();
    SetInt("position_texture", 0);
    SetInt("normal_texture", 1);
    SetInt("albedo_texture", 2);
    SetInt("orm_texture", 3);
    
    SetInt("skybox_texture", 4);
    SetInt("skybox_diffuse_irradiance_texture", 5);
    SetInt("skybox_prefilter_texture", 6);
    SetInt("skybox_brdf_lut_texture", 7);
    
    SetInt("shadow_map", 8);
    for(unsigned int i = 0; i < 4; ++i)
    {
        stringstream ss;
        ss << "shadow_map_pointlight[" << i << "]";
        SetInt(ss.str(), 9 + i);
    }
}

void DeferredBRDFShader::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
	const SRenderInfo& render_info = mesh.GetRenderInfo();
	glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO

	GLuint null_tex_id = CRender::GetNullTexId();
	SetVec3("cam_pos", scene_render_info.camera_pos);
	// todo: 天空盒贴图需要每次都更新吗？
	// 添加天空盒贴图
	glActiveTexture(GL_TEXTURE0 + 4);
	GLuint skybox_tex_id = CRender::GetRender()->GetSkyboxTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex_id);
	// 天空盒辐照度贴图
	glActiveTexture(GL_TEXTURE0 + 5);
	GLuint skybox_irradiance_tex_id = CRender::GetRender()->GetSkyboxDiffuseIrradianceTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_irradiance_tex_id);
	// 天空盒预滤波贴图
	glActiveTexture(GL_TEXTURE0 + 6);
	GLuint skybox_prefilter_tex_id = CRender::GetRender()->GetSkyboxPrefilterTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_prefilter_tex_id);
	// 天空盒brdf lut贴图
	glActiveTexture(GL_TEXTURE0 + 7);
	GLuint skybox_brdf_lut_tex_id = CRender::GetRender()->GetSkyboxBRDFLutTexture();
	glBindTexture(GL_TEXTURE_2D, skybox_brdf_lut_tex_id);
	// 添加光源的阴影贴图
	bool has_dir_light = !scene_render_info.scene_dirlight.expired();
	GLuint dir_light_shadowmap_id = null_tex_id;
	glActiveTexture(GL_TEXTURE0 + 8);
	if(has_dir_light)
	{
		auto dir_light = scene_render_info.scene_dirlight.lock();
		// 支持一个平行光源的阴影贴图
		dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
	
	}
	glBindTexture(GL_TEXTURE_2D,  dir_light_shadowmap_id);

	int point_light_num = 0;
	for(auto light : scene_render_info.scene_pointlights)
	{
		// 限定4个点光源
		if(point_light_num > 3)
		{
			break;
		}
		if(light.expired())
		{
			continue;
		}
		
		glActiveTexture(GL_TEXTURE0 + 9 + point_light_num);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.lock()->GetShadowMapTexture());
		point_light_num++;
	}
}
