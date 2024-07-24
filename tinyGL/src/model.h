#pragma once
#include "renderobj.h"

namespace tinyGL
{
class TGAImage;
class CModel : public CRenderObj
{
public:
	CModel(const SRenderResourceDesc& render_resource_desc);
	
private:
	virtual void GenerateRenderInfo() override;
};
}