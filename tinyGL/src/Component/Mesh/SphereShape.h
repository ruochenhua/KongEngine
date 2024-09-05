#pragma once
#include "MeshComponent.h"

namespace Kong
{
    class SphereShape : public CMeshComponent
    {
    public:
        SphereShape();

    private:
        static std::string sphere_model_path;
    };
}