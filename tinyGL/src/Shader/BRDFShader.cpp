#include "BRDFShader.h"

#include "LightComponent.h"
#include "render.h"
#include "Scene.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

void BRDFShader::SetupData(CMesh& mesh)
{
    auto& render_info = mesh.m_RenderInfo;
	std::vector<float> vertices = mesh.GetVertices();

	glGenVertexArrays(1, &render_info.vertex_array_id);
	glBindVertexArray(render_info.vertex_array_id);

	//init vertex buffer
	glGenBuffers(1, &render_info.vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, render_info.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	//vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,  render_info.vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);

	//normal buffer
	std::vector<float> normals = mesh.GetNormals();
	glGenBuffers(1, &render_info.normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, render_info.normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), &normals[0], GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3,GL_FLOAT,GL_FALSE,0, (void*)0);
	glEnableVertexAttribArray(1);

	// texcoord
	std::vector<float> tex_coords = mesh.GetTextureCoords();
	glGenBuffers(1, &render_info.texture_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, render_info.texture_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,(void*)0);
	glEnableVertexAttribArray(2);
	
	// index buffer
	std::vector<unsigned int> indices = mesh.GetIndices();
	if(!indices.empty())
	{
		glGenBuffers(1, &render_info.index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_info.index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(GL_NONE);

	// m_RenderInfo._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
	render_info.vertex_size = vertices.size();
	render_info.indices_count = indices.size();
}

void BRDFShader::UpdateRenderData(const CMesh& mesh,
			const glm::mat4& actor_model_mat,
			const SSceneRenderInfo& scene_render_info)
{
	const SRenderInfo& render_info = mesh.GetRenderInfo();
	glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO

	// 材质属性
	SetVec3("albedo", render_info.material.albedo);
	SetFloat("specular_factor", render_info.material.specular_factor);
	SetFloat("metallic", render_info.material.metallic);
	SetFloat("roughness", render_info.material.roughness);
	SetFloat("ao", render_info.material.ao);

	/*
	法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
	Normal = mat3(transpose(inverse(model))) * aNormal;
	 */
	// mat3 normal_model_mat = transpose(inverse(actor_model_mat));
	SetMat3("normal_model_mat", actor_model_mat);

	GLuint null_tex_id = CRender::GetNullTexId();
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	glActiveTexture(GL_TEXTURE1);
	GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, specular_map_id);	
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
	Use();
	SetInt("diffuse_texture", 0);
	SetInt("specular_texture", 1);
}

void BRDFShader_NormalMap::SetupData(CMesh& mesh)
{
	BRDFShader::SetupData(mesh);
	auto& render_info = mesh.m_RenderInfo;
	
	glBindVertexArray(render_info.vertex_array_id);
	// tangent
	std::vector<float> tangents = mesh.GetTangents();
	if(!tangents.empty())
	{
		glGenBuffers(1, &render_info.tangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_info.tangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tangents.size(), &tangents[0], GL_STATIC_DRAW);
		glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(3);
	}

	// bitangent
	std::vector<float> bitangents = mesh.GetBitangents();
	if(!bitangents.empty())
	{
		glGenBuffers(1, &render_info.bitangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_info.bitangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*bitangents.size(), &bitangents[0], GL_STATIC_DRAW);
		glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(4);
		glBindVertexArray(GL_NONE);
	}
}

void BRDFShader_NormalMap::UpdateRenderData(const CMesh& mesh, const glm::mat4& actor_model_mat,
	const SSceneRenderInfo& scene_render_info)
{
	BRDFShader::UpdateRenderData(mesh, actor_model_mat, scene_render_info);
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

void BRDFShader_ShadowMap::SetupData(CMesh& mesh)
{
	BRDFShader_NormalMap::SetupData(mesh);
}

void BRDFShader_ShadowMap::UpdateRenderData(const CMesh& mesh, const glm::mat4& actor_model_mat,
	const SSceneRenderInfo& scene_render_info)
{
	BRDFShader_NormalMap::UpdateRenderData(mesh, actor_model_mat, scene_render_info);
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
	
	int point_light_count = 0;
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(point_light_count > 3)
		{
			break;
		}
				
		if(light.expired())
		{
			continue;
		}
		// 先支持一个点光源的阴影贴图
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.lock()->GetShadowMapTexture());
		break;		
		++point_light_count;
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

