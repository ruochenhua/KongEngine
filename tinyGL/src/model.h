#pragma once
#include "renderobj.h"

class TGAImage;
class CModel : public CRenderObj
{
public:
	int ImportObj(const std::string& model_path, const std::string& diffuse_tex_path);
// 	std::vector<float> GetVertices() const;
// 	std::vector<float> GetTextureCoords() const;
// 	std::vector<float> GetNormals() const;
// 	TGAImage* GetTextureImage() const;

private:
// 	std::vector<glm::vec3> m_vVertex;
// 	std::vector<glm::vec3> m_vNormal;
// 	std::vector<glm::vec2> m_vTexCoord;
// 
// 	TGAImage* m_pDiffuseTex = nullptr;
// 
// 	std::vector<unsigned int> m_vIndex;
// 	//information for rendering
// 	SRenderInfo m_RenderInfo;
};