#include "model.h"

#include "render.h"
#include "tgaimage.h"

using namespace glm;
using namespace tinyGL;

CModel::CModel(const SRenderResourceDesc& render_resource_desc)
	:CRenderObj(render_resource_desc)
{
	ImportObj(render_resource_desc.model_path);
	GenerateRenderInfo();
}

void CModel::GenerateRenderInfo()
{
	for(auto& mesh : mesh_list)
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

		// if (m_RenderInfo.diffuse_tex_id)
		// {		
		// 	glBindTexture(GL_TEXTURE_2D, m_RenderInfo.diffuse_tex_id);		
		// }
	

		// tangent
		vector<float> tangents = mesh.GetTangents();
		glGenBuffers(1, &render_info.tangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_info.tangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tangents.size(), &tangents[0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(3);
	
		// bitangent
		vector<float> bitangents = mesh.GetBitangents();
		glGenBuffers(1, &render_info.bitangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_info.bitangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*bitangents.size(), &bitangents[0], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(4);

		// index buffer
		std::vector<unsigned int> indices = mesh.GetIndices();
		glGenBuffers(1, &render_info.index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_info.index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);

	
		glBindVertexArray(GL_NONE);
	
		// m_RenderInfo._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
		render_info.vertex_size = vertices.size();
		render_info.indices_count = indices.size();

		glUseProgram(shader_id);
		glUniform1i(glGetUniformLocation(shader_id, "diffuse_texture"), 0);
		glUniform1i(glGetUniformLocation(shader_id, "specular_texture"), 1);
		glUniform1i(glGetUniformLocation(shader_id, "normal_texture"), 2);
		glUniform1i(glGetUniformLocation(shader_id, "tangent_texture"), 3);
	}
}
