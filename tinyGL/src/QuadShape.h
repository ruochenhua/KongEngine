#pragma once
#include "MeshComponent.h"

namespace tinyGL
{
    class CQuadShape : public CMeshComponent
    {
    public:
        CQuadShape(const SRenderResourceDesc& render_resource_desc);
        virtual void Draw() override;
    private:
        void InitQuadData(const SRenderResourceDesc& render_resource_desc);
        
        std::string texture_path;
        std::string specular_map_path;

    };
}