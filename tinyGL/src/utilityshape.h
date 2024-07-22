#pragma once
#include "renderobj.h"

namespace tinyGL
{
class CUtilityBox : public CRenderObj
{
public:
	CUtilityBox(const vector<string>& shader_path_list);
	virtual std::vector<float> GetVertices() const override;
	virtual std::vector<unsigned int> GetIndices() const override;

	virtual void GenerateRenderInfo() override;
private:
	static std::vector<float> s_vBoxVertices;
	static std::vector<int> s_vBoxIndices;

	std::string texture_path;
	std::string specular_map_path;

	TGAImage* specular_map_image = nullptr;
};
}