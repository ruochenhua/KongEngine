#include "BRDFShader.h"

#include "LightComponent.h"
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
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	glActiveTexture(GL_TEXTURE1);
	GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, specular_map_id);

	// normal map加一个法线贴图的数据
	glActiveTexture(GL_TEXTURE2);
	GLuint normal_tex_id = render_info.normal_tex_id != 0 ? render_info.normal_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, normal_tex_id);

	// glActiveTexture(GL_TEXTURE3);
	// GLuint diffuse_roughness = render_info.diffuse_roughness_tex_id != 0 ? render_info.diffuse_roughness_tex_id : null_tex_id;
	// glBindTexture(GL_TEXTURE_2D, diffuse_roughness);

	// 添加光源的阴影贴图
	bool has_dir_light = !scene_render_info.scene_dirlight.expired();
	GLuint dir_light_shadowmap_id = null_tex_id;
	glActiveTexture(GL_TEXTURE3);
	if(has_dir_light)
	{
		auto dir_light = scene_render_info.scene_dirlight.lock();
		// 支持一个平行光源的阴影贴图
		dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
	
	}
	glBindTexture(GL_TEXTURE_2D,  dir_light_shadowmap_id);
	
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(light.expired())
		{
			continue;
		}
		// 先支持一个点光源的阴影贴图
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.lock()->GetShadowMapTexture());
		break;		
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
	SetInt("diffuse_texture", 0);
	SetInt("specular_texture", 1);
	SetInt("normal_texture", 2);
	SetInt("shadow_map", 3);
	SetInt("shadow_map_pointlight", 4);
}

void BRDFShader_NormalMap::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
	BRDFShader::UpdateRenderData(mesh, scene_render_info);
	GLuint null_tex_id = CRender::GetNullTexId();
	auto& render_info = mesh.m_RenderInfo;
	// normal map加一个法线贴图的数据
	glActiveTexture(GL_TEXTURE2);
	GLuint normal_tex_id = render_info.normal_tex_id != 0 ? render_info.normal_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, normal_tex_id);

	glActiveTexture(GL_TEXTURE3);
	GLuint diffuse_roughness = render_info.diffuse_roughness_tex_id != 0 ? render_info.diffuse_roughness_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_roughness);
}

void BRDFShader_NormalMap::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/brdf_normalmap.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/brdf_normalmap.frag")},
	};
	shader_id = Shader::LoadShaders(shader_path_map);
    
	assert(shader_id, "Shader load failed!");

	// 一些shader的数据绑定
	Use();
	SetInt("diffuse_texture", 0);
	SetInt("specular_texture", 1);
	SetInt("normal_texture", 2);
	SetInt("diffuse_roughness_tex",3);
}

void BRDFShader_ShadowMap::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
	BRDFShader_NormalMap::UpdateRenderData(mesh, scene_render_info);
	GLuint null_tex_id = CRender::GetNullTexId();
	
	// 添加光源的阴影贴图
	bool has_dir_light = !scene_render_info.scene_dirlight.expired();
	GLuint dir_light_shadowmap_id = null_tex_id;
	glActiveTexture(GL_TEXTURE3);
	if(has_dir_light)
	{
		auto dir_light = scene_render_info.scene_dirlight.lock();
		// 支持一个平行光源的阴影贴图
		dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
	
	}
	glBindTexture(GL_TEXTURE_2D,  dir_light_shadowmap_id);
	
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(light.expired())
		{
			continue;
		}
		// 先支持一个点光源的阴影贴图
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.lock()->GetShadowMapTexture());
		break;		
	}
}

void BRDFShader_ShadowMap::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/brdf_shadowmap.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/brdf_shadowmap.frag")},
	};
	shader_id = Shader::LoadShaders(shader_path_map);
    
	assert(shader_id, "Shader load failed!");

	// 一些shader的数据绑定
	Use();
	SetInt("diffuse_texture", 0);
	SetInt("specular_texture", 1);
	SetInt("normal_texture", 2);
	SetInt("shadow_map", 3);
	SetInt("shadow_map_pointlight", 4);
}

