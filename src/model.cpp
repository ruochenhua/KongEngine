#include "model.h"
#include "OBJ_Loader.h"

using namespace glm;

int CModel::ImportObj(const std::string& model_path)
{
	objl::Loader obj_loader;
	bool loadout = obj_loader.LoadFile(model_path);
	if (!loadout)
		return -1;
	
	auto load_meshes = obj_loader.LoadedMeshes;
	auto load_vertices = obj_loader.LoadedVertices;

	m_vVertex.clear();
	m_vVertex.resize(load_vertices.size());

	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		auto &pos = load_vertices[i].Position;
		m_vVertex[i] = vec3(pos.X, pos.Y, pos.Z);
	}

	return 0;
}

std::vector<float> CModel::GetVertices() const
{
	std::vector<float> vertices(m_vVertex.size() * 3);

	for(size_t i = 0; i < m_vVertex.size(); ++i)
	{
		vertices[3 * i] = m_vVertex[i].x;
		vertices[3 * i+1] = m_vVertex[i].y;
		vertices[3 * i+2] = m_vVertex[i].z;
	}

	return vertices;
}