#include "BRDFShader.h"

#include "Component/LightComponent.h"
#include "render.h"
#include "Scene.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

void BRDFShader::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
	const SRenderInfo& render_info = mesh.GetRenderInfo();
	glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO

	// 材质属性
	SetVec4("albedo", render_info.material.albedo);
	SetFloat("specular_factor", render_info.material.specular_factor);
	SetFloat("metallic", render_info.material.metallic);
	SetFloat("roughness", render_info.material.roughness);
	SetFloat("ao", render_info.material.ao);

	GLuint null_tex_id = CRender::GetNullTexId();
	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEX_SHADER_ID);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	// normal map加一个法线贴图的数据
	glActiveTexture(GL_TEXTURE0 + NORMAL_TEX_SHADER_ID);
	GLuint normal_tex_id = render_info.normal_tex_id != 0 ? render_info.normal_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, normal_tex_id);

	glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEX_SHADER_ID);
	GLuint roughness_tex_id = render_info.roughness_tex_id != 0 ? render_info.roughness_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, roughness_tex_id);

	glActiveTexture(GL_TEXTURE0 + METALLIC_TEX_SHADER_ID);
	GLuint metallic_tex_id = render_info.metallic_tex_id != 0 ? render_info.metallic_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, metallic_tex_id);

	glActiveTexture(GL_TEXTURE0 + AO_TEX_SHADER_ID);
	GLuint ao_tex_id = render_info.ao_tex_id != 0 ? render_info.ao_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, ao_tex_id);

	// 添加光源的阴影贴图
	bool has_dir_light = !scene_render_info.scene_dirlight.expired();
	GLuint dir_light_shadowmap_id = null_tex_id;
	glActiveTexture(GL_TEXTURE0 + DIRLIGHT_SM_TEX_SHADER_ID);
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
		
		glActiveTexture(GL_TEXTURE0 + POINTLIGHT_SM_TEX_SHADER_ID + point_light_num);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.lock()->GetShadowMapTexture());
		point_light_num++;
	}
}

void BRDFShader::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/brdf.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/brdf.frag")},
	};
	shader_id = Shader::LoadShaders(shader_path_map);
    
	assert(shader_id, "Shader load failed!");

	// 一些shader的数据绑定
	// ?可能不放这里比较好
	// 一些shader的数据绑定
	Use();
	SetInt("diffuse_texture", DIFFUSE_TEX_SHADER_ID);
	SetInt("normal_texture", NORMAL_TEX_SHADER_ID);
	SetInt("roughness_texture", ROUGHNESS_TEX_SHADER_ID);
	SetInt("metallic_texture", METALLIC_TEX_SHADER_ID);
	SetInt("ao_texture", AO_TEX_SHADER_ID);
	
	SetInt("shadow_map", DIRLIGHT_SM_TEX_SHADER_ID);
	for(unsigned int i = 0; i < 4; ++i)
	{
		stringstream ss;
		ss << "shadow_map_pointlight[" << i << "]";
		SetInt(ss.str(), POINTLIGHT_SM_TEX_SHADER_ID + i);
	}
}

