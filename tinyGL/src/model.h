#pragma once
#include "MeshComponent.h"

namespace tinyGL
{
class TGAImage;
class CModelMeshComponent : public CMeshComponent
{
public:
	CModelMeshComponent(const SRenderResourceDesc& render_resource_desc);
};
}