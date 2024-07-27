#pragma once
#include "renderobj.h"

namespace tinyGL
{
class CUtilityBox : public CRenderObj
{
public:
	CUtilityBox(const SRenderResourceDesc& render_resource_desc);
	virtual std::vector<float> GetVertices() const;
	virtual std::vector<unsigned int> GetIndices() const;

	virtual void GenerateRenderInfo() override;
private:
	static std::vector<float> s_vBoxVertices;
	static std::vector<int> s_vBoxIndices;

	std::string texture_path;
	std::string specular_map_path;

};
}