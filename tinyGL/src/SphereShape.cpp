#include "SphereShape.h"

using namespace tinyGL;

string SphereShape::sphere_model_path = "Engine/sphere/sphere.obj";

SphereShape::SphereShape(const SRenderResourceDesc& render_resource_desc)
    : CMeshComponent(render_resource_desc)
{
    ImportObj(CSceneLoader::ToResourcePath(sphere_model_path));
    LoadOverloadTexture(render_resource_desc);
}
