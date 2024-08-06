#include "BRDFShader.h"

#include "LightComponent.h"
#include "render.h"
#include "Scene.h"
using namespace tinyGL;
using namespace glm;
using namespace std;
BRDFShader::BRDFShader()
{
    shader_path_map.emplace(EShaderType::vs, CSceneLoader::ToResourcePath("shader/brdf.vert"));
    shader_path_map.emplace(EShaderType::fs, CSceneLoader::ToResourcePath("shader/brdf.frag"));

    shader_id = Shader::LoadShaders(shader_path_map);
    
    assert(shader_id, "Shader load failed!");
}

// todo
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
	if(indices.size() > 0)
	{
		glGenBuffers(1, &render_info.index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_info.index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(GL_NONE);

	// m_RenderInfo._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
	render_info.vertex_size = vertices.size();
	render_info.indices_count = indices.size();

	Use();
	SetInt("diffuse_texture", 0);
	SetInt("specular_texture", 1);
}

void BRDFShader::UpdateRenderData(const CMesh& mesh,
			const glm::mat4& actor_model_mat,
			const SSceneRenderInfo& scene_render_info)
{
	const SRenderInfo& render_info = mesh.GetRenderInfo();
	glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
	
	SetMat4("model", actor_model_mat);
	SetMat4("view", scene_render_info.camera_view);
	SetMat4("proj", scene_render_info.camera_proj);
	SetVec3("cam_pos", scene_render_info.camera_pos);

	// 材质属性
	SetVec3("albedo", render_info.material.albedo);
	SetFloat("metallic", render_info.material.metallic);
	SetFloat("roughness", render_info.material.roughness);
	SetFloat("ao", render_info.material.ao);

	/*
	法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
	Normal = mat3(transpose(inverse(model))) * aNormal;
	 */
	mat3 normal_model_mat = transpose(inverse(actor_model_mat));
	SetMat3("normal_model_mat", normal_model_mat);

	GLuint null_tex_id = CRender::GetNullTexId();
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	glActiveTexture(GL_TEXTURE1);
	GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, specular_map_id);

	if(!scene_render_info.scene_dirlight.expired())
	{
		SetVec3("directional_light.light_dir", scene_render_info.scene_dirlight.lock()->GetLightDir());
		SetVec3("directional_light.light_color", scene_render_info.scene_dirlight.lock()->light_color);
	}
	int point_light_count = 0;
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(light.expired())
		{
			continue;
		}
		
		stringstream point_light_name;
		point_light_name <<  "point_lights[" << point_light_count << "]";
		SetVec3(point_light_name.str() + ".light_pos", light.lock()->GetLightLocation());
		SetVec3(point_light_name.str() + ".light_color", light.lock()->light_color);
		// 先支持一个点光源的阴影贴图
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.lock()->shadowmap_texture);
	
		++point_light_count;
	}
	SetInt("point_light_count", scene_render_info.scene_pointlights.size());

	// Draw the triangle !
	// if no index, use draw array
	
	if(render_info.index_buffer == GL_NONE)
	{
		// Starting from vertex 0; 3 vertices total -> 1 triangle
		glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size); 	
	}
	else
	{		
		glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
	}
}
