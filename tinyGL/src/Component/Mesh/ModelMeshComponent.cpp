#include "ModelMeshComponent.h"
#include "render.h"

using namespace glm;
using namespace Kong;

CModelMeshComponent::CModelMeshComponent(const string& model_path)
{
	ImportObj(model_path);
}

void CModelMeshComponent::Draw(const SSceneRenderInfo& scene_render_info)
{
	CMeshComponent::Draw(scene_render_info);
}
