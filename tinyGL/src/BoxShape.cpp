#include "BoxShape.h"

#include "render.h"

using namespace tinyGL;
using namespace std;
string CBoxShape::box_model_path = "Engine/box/box.obj";

CBoxShape::CBoxShape(const SRenderResourceDesc& render_resource_desc)
    : CMeshComponent(render_resource_desc)
{
    ImportObj(CSceneLoader::ToResourcePath(box_model_path));
    LoadOverloadTexture(render_resource_desc);
}

void CBoxShape::Draw(const SSceneRenderInfo& scene_render_info)
{
    CMeshComponent::Draw(scene_render_info);
}