#pragma once
#include "Shader.h"

namespace Kong
{
    // todo：或许应该改名为pbrshader
    class PBRShader : public Shader
    {
    public:
        PBRShader() = default;
        
        void UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info) override;
    
        virtual void InitDefaultShader() override;
    };
    //
    // // brdf_normalmap 就是brdf的变种，很多能力可以和BRDFShader共用
    // class BRDFShader_NormalMap : public BRDFShader
    // {
    // public:
    //     BRDFShader_NormalMap() = default;
    //     
    //     void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
    //     
    //     virtual void InitDefaultShader() override;
    // };
    //
    // // brdf_shadowmap 是基于brdf_normalmap的演化
    // class BRDFShader_ShadowMap : public BRDFShader_NormalMap
    // {
    // public:
    //     BRDFShader_ShadowMap() = default;
    //     
    //     void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
    //     
    //     virtual void InitDefaultShader() override;
    // };
}