#include "model.h"
#include "tgaimage.h"

using namespace glm;
using namespace tinyGL;

CModel::CModel(const string& model_path, const string& diffuse_tex_path, const vector<string>& shader_path_list)
	:CRenderObj(shader_path_list)
{
	ImportObj(model_path);
	m_RenderInfo.diffuse_tex_id = LoadTexture(diffuse_tex_path);
	GenerateRenderInfo();
}

void CModel::GenerateRenderInfo()
{
	std::vector<float> vertices = GetVertices();

	glGenVertexArrays(1, &m_RenderInfo.vertex_array_id);
	glBindVertexArray(m_RenderInfo.vertex_array_id);

	//init vertex buffer
	glGenBuffers(1, &m_RenderInfo.vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	//vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,  m_RenderInfo.vertex_buffer);
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
	std::vector<float> normals = GetNormals();
	glGenBuffers(1, &m_RenderInfo.normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo.normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), &normals[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(
		1,                  // attribute 2. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(1);
		
	if (m_RenderInfo.diffuse_tex_id)
	{
		std::vector<float> tex_coords = GetTextureCoords();
		glGenBuffers(1, &m_RenderInfo.texture_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo.texture_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
	
		glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo.texture_buffer);
		glVertexAttribPointer(
			2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		glEnableVertexAttribArray(2);
		
		// glActiveTexture(GL_TEXTURE0);	//���ֻ��һ��texture�Ļ����Բ�д�����texture����shader�Ļ���Ҫ���ò�ͬ��activate texture
		glBindTexture(GL_TEXTURE_2D, m_RenderInfo.diffuse_tex_id);		
	}
	// index buffer
	std::vector<unsigned int> indices = GetIndices();
	glGenBuffers(1, &m_RenderInfo.index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderInfo.index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
	
	glBindVertexArray(GL_NONE);
	
	// m_RenderInfo._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
	m_RenderInfo.vertex_size = vertices.size();
	m_RenderInfo.indices_count = indices.size();
}
