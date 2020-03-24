#include "model.h"
#include "OBJ_Loader.h"
#include "tgaimage.h"

namespace tinyGL
{
using namespace glm;

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

std::vector<float> CModel::GetVertices() const
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

std::vector<float> CModel::GetTextureCoords() const
{
	std::vector<float> tex_coords(m_vTexCoord.size() * 2);
	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		tex_coords[2 * i] = m_vTexCoord[i].x;
		tex_coords[2 * i + 1] = 1.0 - m_vTexCoord[i].y;	//to opengl, invert y coord
	}

	return tex_coords;
}

std::vector<float> CModel::GetNormals() const
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

TGAImage* CModel::GetTextureImage() const
{
	return m_pDiffuseTex;
}
}