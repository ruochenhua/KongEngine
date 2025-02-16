#pragma once
#include "MeshComponent.h"

namespace Kong
{
    class CQuadShape : public CMeshComponent
    {
    public:
        CQuadShape();
        void Draw() override;
        virtual void InitRenderInfo() override;

        void BindVAO();
    };
}