#include "model.h"
#include "render.h"

using namespace glm;
using namespace tinyGL;

CModelMeshComponent::CModelMeshComponent(const SRenderResourceDesc& render_resource_desc)
	:CMeshComponent(render_resource_desc)
{
	ImportObj(render_resource_desc.model_path);
	for(auto& mesh : mesh_list)
	{
		shader_data->SetupData(mesh);	
	}
}
