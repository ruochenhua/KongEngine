#include "renderobj.h"
#include "OBJ_Loader.h"
#include "tgaimage.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"

using namespace tinyGL;
using namespace glm;

CRenderObj::CRenderObj(const vector<string>& shader_path_list)
{
	// shader list
	// 0:vs, 1:fs, ...
	// todo: dict in yaml
	m_RenderInfo.program_id = Shader::LoadShaders(shader_path_list[0], shader_path_list[1]);
}

std::vector<float> CRenderObj::GetVertices() const
{
	std::vector<float> vertices(m_vVertex.size() * 3);

	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		vertices[3 * i] = m_vVertex[i].x;
		vertices[3 * i + 1] = m_vVertex[i].y;
		vertices[3 * i + 2] = m_vVertex[i].z;
	}

	return vertices;
}

std::vector<float> CRenderObj::GetTextureCoords() const
{
	std::vector<float> tex_coords(m_vTexCoord.size() * 2);
	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		tex_coords[2 * i] = m_vTexCoord[i].x;
		tex_coords[2 * i + 1] = 1.0 - m_vTexCoord[i].y;	//to opengl, invert y coord
	}

	return tex_coords;
}

std::vector<float> CRenderObj::GetNormals() const
{
	std::vector<float> normals(m_vNormal.size() * 3);
	for (size_t i = 0; i < m_vNormal.size(); ++i)
	{
		normals[3 * i] = m_vNormal[i].x;
		normals[3 * i + 1] = m_vNormal[i].y;
		normals[3 * i + 2] = m_vNormal[i].z;
	}

	return normals;
}

int CRenderObj::ImportObj(const std::string& model_path)
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

	return 0;
}

GLuint CRenderObj::LoadTexture(const std::string& texture_path)
{
	GLuint texture_id = GL_NONE;
	if (texture_path.empty())
	{
		return texture_id;		
	}

	int width, height, nr_component;
	auto data = stbi_load(texture_path.c_str(), &width, &height, &nr_component, 0);
	assert(data);
	
	GLenum format = GL_BGR;
	switch(nr_component)
	{
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		break;
	}
	
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// release memory
	stbi_image_free(data);
	// auto tex_image = new TGAImage;
	// tex_image->read_tga_file(texture_path.c_str());
	

	return texture_id;
}

std::vector<unsigned int> CRenderObj::GetIndices() const
{
	return m_vIndex;
}

