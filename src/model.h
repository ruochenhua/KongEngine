#pragma once
#include "common.h"

class CModel
{
public:
	int ImportObj(const std::string& model_path);
	std::vector<float> GetVertices() const;

private:	
	std::vector<glm::vec3> m_vVertex;	//for now, only have positions
	
	//information for rendering 
	SRenderInfo m_RenderInfo;
};