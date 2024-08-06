#include "model.h"

#include "render.h"
#include "tgaimage.h"
#include "Shader/BRDFShader.h"

using namespace glm;
using namespace tinyGL;

CModelMeshComponent::CModelMeshComponent(const SRenderResourceDesc& render_resource_desc)
	:CMeshComponent(render_resource_desc)
{
	ImportObj(render_resource_desc.model_path);
	if(render_resource_desc.shader_type.empty())
	{
		assert(shader_data.get(), "shader_data not created!");
		GenerateDefaultRenderInfo();
	}
}
