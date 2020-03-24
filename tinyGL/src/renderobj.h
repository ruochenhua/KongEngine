#pragma once
#include "common.h"

namespace tinyGL
{
class TGAImage;
class CRenderObj
{
public:
	virtual std::vector<float> GetVertices() const;
	virtual std::vector<float> GetTextureCoords() const;
	virtual std::vector<float> GetNormals() const;
	virtual TGAImage* GetTextureImage() const;

protected:
	std::vector<glm::vec3> m_vVertex;
	std::vector<glm::vec3> m_vNormal;
	std::vector<glm::vec2> m_vTexCoord;

	TGAImage* m_pDiffuseTex = nullptr;

	std::vector<unsigned int> m_vIndex;

	SRenderInfo m_RenderInfo;
};
}