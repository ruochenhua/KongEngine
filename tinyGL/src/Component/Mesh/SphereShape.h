#pragma once
#include "MeshComponent.h"

namespace tinyGL
{
    class SphereShape : public CMeshComponent
    {
    public:
        SphereShape(const SRenderResourceDesc& render_resource_desc);

    private:
        static std::string sphere_model_path;
    };
}