#include "ModelMeshComponent.h"
#include "render.h"

using namespace glm;
using namespace tinyGL;

CModelMeshComponent::CModelMeshComponent(const SRenderResourceDesc& render_resource_desc)
	:CMeshComponent(render_resource_desc)
{
	ImportObj(render_resource_desc.model_path);
	// texture overload
	LoadOverloadTexture(render_resource_desc);
}

void CModelMeshComponent::Draw(const SSceneRenderInfo& scene_render_info)
{
	CMeshComponent::Draw(scene_render_info);
}
