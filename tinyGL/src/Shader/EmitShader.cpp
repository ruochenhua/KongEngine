#include "EmitShader.h"

#include "render.h"
#include "Scene.h"
using namespace tinyGL;
using namespace glm;
using namespace std;


// todo
void EmitShader::SetupData(CMesh& mesh)
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

void EmitShader::UpdateRenderData(const CMesh& mesh,
			const SSceneRenderInfo& scene_render_info)
{
	const SRenderInfo& render_info = mesh.GetRenderInfo();
	glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO

	// 材质属性
	SetVec3("albedo", render_info.material.albedo);
	
	GLuint null_tex_id = CRender::GetNullTexId();
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	glActiveTexture(GL_TEXTURE1);
	GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, specular_map_id);
}

void EmitShader::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/emit.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/emit.frag")},
	};
	shader_id = Shader::LoadShaders(shader_path_map);
    
	assert(shader_id, "Shader load failed!");
}
