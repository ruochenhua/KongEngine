#include "model.h"
#include "OBJ_Loader.h"
#include "tgaimage.h"

using namespace glm;
using namespace tinyGL;

CModel::CModel(const std::string& model_path, const std::string& diffuse_tex_path)
{
	ImportObj(model_path, diffuse_tex_path);
	GenerateRenderInfo();
}


int CModel::ImportObj(const std::string& model_path, const std::string& diffuse_tex_coord)
{
	objl::Loader obj_loader;

	bool loadout = obj_loader.LoadFile(model_path);
	if (!loadout)
		return -1;

	auto load_meshes = obj_loader.LoadedMeshes;
	auto load_vertices = obj_loader.LoadedVertices;
	auto load_materials = obj_loader.LoadedMaterials;

	m_vIndex = obj_loader.LoadedIndices;

	size_t vertex_count = load_vertices.size();

	m_vVertex.clear();
	m_vVertex.resize(vertex_count);

	m_vTexCoord.clear();
	m_vTexCoord.resize(vertex_count);

	m_vNormal.clear();
	m_vNormal.resize(vertex_count);

	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		auto &pos = load_vertices[i].Position;
		m_vVertex[i] = vec3(pos.X, pos.Y, pos.Z);

		auto tex_coord = load_vertices[i].TextureCoordinate;
		m_vTexCoord[i] = vec2(tex_coord.X, tex_coord.Y);

		auto normal = load_vertices[i].Normal;
		m_vNormal[i] = vec3(normal.X, normal.Y, normal.Z);
	}

	if (!diffuse_tex_coord.empty())
	{
		if (m_pDiffuseTex)
		{
			delete m_pDiffuseTex;
			m_pDiffuseTex = nullptr;
		}

		m_pDiffuseTex = new TGAImage;
		m_pDiffuseTex->read_tga_file(diffuse_tex_coord.c_str());
	}

	return 0;
}

void CModel::GenerateRenderInfo()
{
	std::vector<float> vertices = GetVertices();

	glGenVertexArrays(1, &m_RenderInfo.vertexArrayId);
	glBindVertexArray(m_RenderInfo.vertexArrayId);

	//init vertex buffer
	glGenBuffers(1, &m_RenderInfo.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	//vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,  m_RenderInfo.vertexBuffer);
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
	glGenBuffers(1, &m_RenderInfo._normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo._normal_buffer);
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
	
	TGAImage* tex_img = GetTextureImage();
	if (tex_img)
	{
		std::vector<float> tex_coords = GetTextureCoords();
		glGenBuffers(1, &m_RenderInfo._texture_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo._texture_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
	
		glBindBuffer(GL_ARRAY_BUFFER, m_RenderInfo._texture_buffer);
		glVertexAttribPointer(
			2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		glEnableVertexAttribArray(2);
		
		glGenTextures(1, &m_RenderInfo.texture_id);
		// glActiveTexture(GL_TEXTURE0);	//如果只有一个texture的话可以不写，多个texture传入shader的话就要设置不同的activate texture
		glBindTexture(GL_TEXTURE_2D, m_RenderInfo.texture_id);

		int tex_width = tex_img->get_width();
		int tex_height = tex_img->get_height();
		unsigned char* tex_data = tex_img->buffer();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, (void*)tex_data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	// index buffer
	std::vector<unsigned int> indices = GetIndices();
	glGenBuffers(1, &m_RenderInfo.indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderInfo.indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
	glBindVertexArray(GL_NONE);
	
	// m_RenderInfo._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
	m_RenderInfo._vertex_size = vertices.size();
	m_RenderInfo._indices_count = indices.size();
	m_RenderInfo._texture_img = GetTextureImage();
}
