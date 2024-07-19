#pragma once
#include "renderobj.h"

namespace tinyGL
{
class TGAImage;
class CModel : public CRenderObj
{
public:
	CModel(const std::string& model_path, const std::string& diffuse_tex_path);
	
private:
	virtual void GenerateRenderInfo() override;
};
}