#include "PBRShader.h"

#include "Component/LightComponent.h"
#include "render.h"
#include "Scene.h"

using namespace Kong;
using namespace glm;
using namespace std;

void PBRShader::UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info)
{	
	// 材质属性
	SetVec4("albedo", render_material.albedo);
	SetFloat("specular_factor", render_material.specular_factor);
	SetFloat("metallic", render_material.metallic);
	SetFloat("roughness", render_material.roughness);
	SetFloat("ao", render_material.ao);

	GLuint null_tex_id = CRender::GetNullTexId();
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

	// todo: 天空盒贴图需要每次都更新吗？
	// 添加天空盒贴图
	glActiveTexture(GL_TEXTURE0 + SKYBOX_TEX_SHADER_ID);
	GLuint skybox_tex_id = CRender::GetRender()->GetSkyboxTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex_id);
	// 天空盒辐照度贴图
	glActiveTexture(GL_TEXTURE0 + SKYBOX_DIFFUSE_IRRADIANCE_TEX_SHADER_ID);
	GLuint skybox_irradiance_tex_id = CRender::GetRender()->GetSkyboxDiffuseIrradianceTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_irradiance_tex_id);
	// 天空盒预滤波贴图
	glActiveTexture(GL_TEXTURE0 + SKYBOX_PREFILTER_TEX_SHADER_ID);
	GLuint skybox_prefilter_tex_id = CRender::GetRender()->GetSkyboxPrefilterTexture();
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_prefilter_tex_id);
	// 天空盒brdf lut贴图
	glActiveTexture(GL_TEXTURE0 + SKYBOX_BRDF_LUT_TEX_SHADER_ID);
	GLuint skybox_brdf_lut_tex_id = CRender::GetRender()->GetSkyboxBRDFLutTexture();
	glBindTexture(GL_TEXTURE_2D, skybox_brdf_lut_tex_id);
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
			SetInt("light_space_matrix_count", dir_light->light_space_matrices.size());
#endif
			if(dir_light->enable_rsm)
			{
				rsm_world_pos = dir_light->rsm_world_position;
				rsm_world_normal = dir_light->rsm_world_normal;
				rsm_world_flux = dir_light->rsm_world_flux;
			}
		}
	}
	
#if USE_CSM
	glActiveTexture(GL_TEXTURE0 + DIRLIGHT_SM_TEX_SHADER_ID);
	glBindTexture(GL_TEXTURE_2D_ARRAY,  dir_light_shadowmap_id);
#else
	glActiveTexture(GL_TEXTURE0 + DIRLIGHT_SM_TEX_SHADER_ID);
	glBindTexture(GL_TEXTURE_2D,  dir_light_shadowmap_id);
#endif
	glActiveTexture(GL_TEXTURE0 + DIRLIGHT_RSM_WORLD_POS);
	glBindTexture(GL_TEXTURE_2D, rsm_world_pos);
	glActiveTexture(GL_TEXTURE0 + DIRLIGHT_RSM_WORLD_NORMAL);
	glBindTexture(GL_TEXTURE_2D, rsm_world_normal);
	glActiveTexture(GL_TEXTURE0 + DIRLIGHT_RSM_WORLD_FLUX);
	glBindTexture(GL_TEXTURE_2D, rsm_world_flux);
	
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
		
		glActiveTexture(GL_TEXTURE0 + POINTLIGHT_SM_TEX_SHADER_ID + point_light_shadow_num);
		glBindTexture(GL_TEXTURE_CUBE_MAP, point_light_ptr->GetShadowMapTexture());
		point_light_shadow_num++;
	}
}

void PBRShader::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/brdf.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/brdf.frag")},
	};
	shader_id = Shader::LoadShaders(shader_path_map);
    
	assert(shader_id, "Shader load failed!");

	// 一些shader的数据绑定
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
	SetInt("shadow_map", DIRLIGHT_SM_TEX_SHADER_ID);
	SetInt("rsm_world_pos", DIRLIGHT_RSM_WORLD_POS);
	SetInt("rsm_world_normal", DIRLIGHT_RSM_WORLD_NORMAL);
	SetInt("rsm_world_flux", DIRLIGHT_RSM_WORLD_FLUX);
	
	for(unsigned int i = 0; i < POINT_LIGHT_SHADOW_MAX; ++i)
	{
		stringstream ss;
		ss << "shadow_map_pointlight[" << i << "]";
		SetInt(ss.str(), POINTLIGHT_SM_TEX_SHADER_ID + i);
	}
}

