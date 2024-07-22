#pragma once
#include "renderobj.h"

namespace tinyGL
{
class TGAImage;
class CModel : public CRenderObj
{
public:
	CModel(const string& model_path,
		const string& diffuse_tex_path,
		const vector<string>& shader_path_list);
	
private:
	virtual void GenerateRenderInfo() override;
};
}