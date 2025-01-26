#include "DeferInfoShader.h"

#include "Render/RenderModule.hpp"
#include "Scene.hpp"
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
    SetInt("diffuse_texture", DIFFUSE_TEX_SHADER_ID);
    SetInt("normal_texture", NORMAL_TEX_SHADER_ID);
    SetInt("roughness_texture", ROUGHNESS_TEX_SHADER_ID);
    SetInt("metallic_texture", METALLIC_TEX_SHADER_ID);
    SetInt("ao_texture", AO_TEX_SHADER_ID);
    // SetInt("skybox_texture", SKYBOX_TEX_SHADER_ID);
    // SetInt("skybox_diffuse_irradiance_texture", SKYBOX_DIFFUSE_IRRADIANCE_TEX_SHADER_ID);
    // SetInt("skybox_prefilter_texture", SKYBOX_PREFILTER_TEX_SHADER_ID);
    // SetInt("skybox_brdf_lut_texture", SKYBOX_BRDF_LUT_TEX_SHADER_ID);
}

void DeferInfoShader::UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info)
{
	// 材质属性
	SetVec4("albedo", render_material.albedo);
	SetFloat("specular_factor", render_material.specular_factor);
	SetFloat("metallic", render_material.metallic);
	SetFloat("roughness", render_material.roughness);
	SetFloat("ao", render_material.ao);
	
	GLuint null_tex_id = KongRenderModule::GetNullTexId();
	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEX_SHADER_ID);
	GLuint diffuse_tex_id = render_material.diffuse_tex_id != 0 ? render_material.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	// normal map加一个法线贴图的数据
	glActiveTexture(GL_TEXTURE0 + NORMAL_TEX_SHADER_ID);
	GLuint normal_tex_id = render_material.normal_tex_id != 0 ? render_material.normal_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, normal_tex_id);

	glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEX_SHADER_ID);
	GLuint roughness_tex_id = render_material.roughness_tex_id != 0 ? render_material.roughness_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, roughness_tex_id);

	glActiveTexture(GL_TEXTURE0 + METALLIC_TEX_SHADER_ID);
	GLuint metallic_tex_id = render_material.metallic_tex_id != 0 ? render_material.metallic_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, metallic_tex_id);

	glActiveTexture(GL_TEXTURE0 + AO_TEX_SHADER_ID);
	GLuint ao_tex_id = render_material.ao_tex_id != 0 ? render_material.ao_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, ao_tex_id);
}

void DeferredBRDFShader::InitDefaultShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/defer_pbr.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/defer_pbr.frag")},
    };
    shader_id = Shader::LoadShaders(shader_path_map);
    
    assert(shader_id, "Shader load failed!");
	
    SetInt("position_texture", 0);
    SetInt("normal_texture", 1);
    SetInt("albedo_texture", 2);
    SetInt("orm_texture", 3);
    
    SetInt("skybox_texture", 4);
    SetInt("skybox_diffuse_irradiance_texture", 5);
    SetInt("skybox_prefilter_texture", 6);
    SetInt("skybox_brdf_lut_texture", 7);
    
    SetInt("shadow_map", 8);
	SetInt("rsm_world_pos", 9);
	SetInt("rsm_world_normal", 10);
	SetInt("rsm_world_flux", 11);
	
    for(unsigned int i = 0; i < 4; ++i)
    {
        stringstream ss;
        ss << "shadow_map_pointlight[" << i << "]";
        SetInt(ss.str(), 12 + i);
    }

	// ssao结果数据
	SetInt("ssao_result_texture", 16);
}

void DeferredBRDFShader::UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info)
{
	GLuint null_tex_id = KongRenderModule::GetNullTexId();
	int texture_idx = 4;
	// 贴图0-3分别是position/normal/albedo/orm, 下面的从4开始算
	// todo: 天空盒贴图需要每次都更新吗？整理一下贴图对应的index吧
	
#if USE_DSA
	auto& render_module = KongRenderModule::GetRenderModule();
	glBindTextureUnit(texture_idx++, render_module.GetSkyboxTexture());
	glBindTextureUnit(texture_idx++, render_module.GetSkyboxDiffuseIrradianceTexture());
	glBindTextureUnit(texture_idx++, render_module.GetSkyboxPrefilterTexture());
	glBindTextureUnit(texture_idx++, render_module.GetSkyboxBRDFLutTexture());
#else
	// 添加天空盒贴图
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	GLuint skybox_tex_id = CRender::GetRender()->GetSkyboxTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex_id);
	// 天空盒辐照度贴图
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	GLuint skybox_irradiance_tex_id = CRender::GetRender()->GetSkyboxDiffuseIrradianceTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_irradiance_tex_id);
	// 天空盒预滤波贴图
	glActiveTexture(GL_TEXTURE0 +	texture_idx++);
	GLuint skybox_prefilter_tex_id = CRender::GetRender()->GetSkyboxPrefilterTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_prefilter_tex_id);
	// 天空盒brdf lut贴图
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	GLuint skybox_brdf_lut_tex_id = CRender::GetRender()->GetSkyboxBRDFLutTexture();
	glBindTexture(GL_TEXTURE_2D, skybox_brdf_lut_tex_id);
#endif
	
	// 添加光源的阴影贴图
	bool has_dir_light = !scene_render_info.scene_dirlight.expired();
	
	GLuint dir_light_shadowmap_id, rsm_world_pos, rsm_world_normal, rsm_world_flux;
	dir_light_shadowmap_id = rsm_world_pos = rsm_world_normal = rsm_world_flux = null_tex_id;
	
	if(has_dir_light)
	{
		auto dir_light = scene_render_info.scene_dirlight.lock();
		if(dir_light->enable_shadowmap)
		{
			// 支持一个平行光源的阴影贴图
			dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
#if USE_CSM
			// csm相关的数据
			for(int i = 0; i < dir_light->csm_distances.size(); ++i)
			{
				stringstream ss;
				ss << "csm_distances[" << i << "]";
				SetFloat(ss.str(), dir_light->csm_distances[i]);
			}
			SetInt("csm_level_count", dir_light->csm_distances.size());
			for(int i = 0; i < dir_light->light_space_matrices.size(); ++i)
			{
				stringstream ss;
				ss << "light_space_matrices[" << i << "]";
				SetMat4(ss.str(), dir_light->light_space_matrices[i]);
			}
#else
			SetMat4("light_space_matrices[0]", dir_light->light_space_mat);
#endif
			// rsm相关信息
			if(dir_light->enable_rsm)
			{
				rsm_world_pos = dir_light->rsm_world_position;
				rsm_world_normal = dir_light->rsm_world_normal;
				rsm_world_flux = dir_light->rsm_world_flux;
			}
		}
	}
	
#if USE_CSM
	
#if USE_DSA
	glBindTextureUnit(texture_idx++, dir_light_shadowmap_id);
#else
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dir_light_shadowmap_id);
#endif
	
#else

#if USE_DSA
	glBindTextureUnit(texture_idx++, dir_light_shadowmap_id);
#else
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	glBindTexture(GL_TEXTURE_2D,  dir_light_shadowmap_id);
#endif

#endif

	// rsm相关贴图数据
#if USE_DSA
	glBindTextureUnit(texture_idx++, rsm_world_pos);
	glBindTextureUnit(texture_idx++, rsm_world_normal);
	glBindTextureUnit(texture_idx++, rsm_world_flux);
#else
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	glBindTexture(GL_TEXTURE_2D, rsm_world_pos);
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	glBindTexture(GL_TEXTURE_2D, rsm_world_normal);
	glActiveTexture(GL_TEXTURE0 + texture_idx++);
	glBindTexture(GL_TEXTURE_2D, rsm_world_flux);
#endif
	
	int point_light_shadow_num = 0;
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(point_light_shadow_num > 3 || light.expired())
		{
			continue;
		}
		auto point_light_ptr = light.lock();
		if(!point_light_ptr->enable_shadowmap)
		{
			continue;
		}
#if USE_DSA
		glBindTextureUnit(texture_idx++, point_light_ptr->GetShadowMapTexture());	
#else
		glActiveTexture(GL_TEXTURE0 + texture_idx++);
		glBindTexture(GL_TEXTURE_CUBE_MAP, point_light_ptr->GetShadowMapTexture());
#endif
		point_light_shadow_num++;
	}
}

DeferredTerrainInfoShader::DeferredTerrainInfoShader()
{
	InitDefaultShader();
}

void DeferredTerrainInfoShader::InitDefaultShader()
{
	Shader::InitDefaultShader();
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.frag")},
		{tcs, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.tesc")},
		{tes, CSceneLoader::ToResourcePath("shader/terrain/terrain_tess.tese")}
	};

    shader_id = LoadShaders(shader_path_map);
	
	SetInt("height_map", 0);
	SetInt("csm", 1);
	SetInt("grass_texture", 2);
	SetInt("grass_normal_texture", 3);
	SetInt("sand_texture", 4);
	SetInt("sand_normal_texture", 5);
	SetInt("rock_texture", 6);
	SetInt("rock_normal_texture", 7);
}

SSAOShader::SSAOShader()
{
	InitDefaultShader();
}


void SSAOShader::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/ssao.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/ssao.frag")},
	};
	shader_id = Shader::LoadShaders(shader_path_map);
    
	assert(shader_id, "Shader load failed!");
	
	SetInt("position_texture", 0);
	SetInt("normal_texture", 1);
	SetInt("noise_texture", 2);
}

SSReflectionShader::SSReflectionShader()
{
	InitDefaultShader();
}

void SSReflectionShader::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/ssr.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/ssr.frag")},
	};

	shader_id = Shader::LoadShaders(shader_path_map);
	assert(shader_id, "Shader load failed!");

	Use();
	SetInt("scene_position", 0);
	SetInt("scene_normal", 1);
	SetInt("scene_color", 2);
	SetInt("orm_texture", 3);
}

