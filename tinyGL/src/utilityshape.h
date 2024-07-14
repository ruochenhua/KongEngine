#pragma once
#include "renderobj.h"

namespace tinyGL
{
class CUtilityBox : public CRenderObj
{
public:
	virtual std::vector<float> GetVertices() const override;
	virtual std::vector<unsigned int> GetIndices() const override;

private:
	static std::vector<float> s_vBoxVertices;
	static std::vector<int> s_vBoxIndices;
};
}