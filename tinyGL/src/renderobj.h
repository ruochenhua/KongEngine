#pragma once
#include "common.h"

namespace tinyGL
{

class SceneObject
{
public:
	glm::mat4 GetModelMatrix() const;
	glm::vec3 location = glm::vec3(0,0,0);
	glm::vec3 rotation = glm::vec3(0,0,0);
	glm::vec3 scale = glm::vec3(1,1,1);
};
	
class CRenderObj : public SceneObject
{
public:
	CRenderObj(const vector<string>& shader_path_list);	
	virtual vector<float> GetVertices() const;
	virtual vector<float> GetTextureCoords() const;
	virtual vector<float> GetNormals() const;
	virtual vector<unsigned int> GetIndices() const;

	SRenderInfo GetRenderInfo() {return m_RenderInfo;}
	
protected:
	virtual void GenerateRenderInfo() = 0;
	
	vector<glm::vec3> m_vVertex;
	vector<glm::vec3> m_vNormal;
	vector<glm::vec2> m_vTexCoord;

	vector<unsigned int> m_vIndex;

	SRenderInfo m_RenderInfo;

	// import obj model
	int ImportObj(const std::string& model_path);
	// load image file and create texture 
	GLuint LoadTexture(const std::string& texture_path);
};
}