#pragma once
#include "MeshComponent.h"

namespace Kong
{
    class CQuadShape : public CMeshComponent
    {
    public:
        CQuadShape();
        virtual void Draw(const SSceneRenderInfo& scene_render_info) override;
        void Draw();
        virtual void InitRenderInfo() override;

        void BindVAO();
    };
}