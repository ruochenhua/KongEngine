#pragma once
/**
 * 渲染公共类型，与 API 无关；不包含 GL/Vulkan 头文件，实现层负责映射。
 * @see Render/Abstraction/Types.hpp 中 ShaderStage、DataFormat 等 RHI 类型
 */
#include <cstdint>
#include <vector>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "Resource/Buffer.hpp"
//#include "Resource/Texture.hpp"

#define SHADOWMAP_DEBUG 0
#define USE_CSM 1

#define USE_COMPUTE_POSTPROCESS 0   // 使用compute shader实现着色

namespace Kong
{
    class KongTexture;
    class CPointLightComponent;
    class CDirectionalLightComponent;

    constexpr unsigned DIFFUSE_TEX_SHADER_ID = 0;
    constexpr unsigned NORMAL_TEX_SHADER_ID = 1;
    constexpr unsigned ROUGHNESS_TEX_SHADER_ID = 2;
    constexpr unsigned METALLIC_TEX_SHADER_ID = 3;
    constexpr unsigned AO_TEX_SHADER_ID = 4;
    constexpr unsigned SKYBOX_TEX_SHADER_ID = 5;
    constexpr unsigned SKYBOX_DIFFUSE_IRRADIANCE_TEX_SHADER_ID = 6;
    constexpr unsigned SKYBOX_PREFILTER_TEX_SHADER_ID = 7;
    constexpr unsigned SKYBOX_BRDF_LUT_TEX_SHADER_ID = 8;
    constexpr unsigned DIRLIGHT_SM_TEX_SHADER_ID = 9;
    constexpr unsigned DIRLIGHT_RSM_WORLD_POS = 10;
    constexpr unsigned DIRLIGHT_RSM_WORLD_NORMAL = 11;
    constexpr unsigned DIRLIGHT_RSM_WORLD_FLUX = 12;
    constexpr unsigned POINTLIGHT_SM_TEX_SHADER_ID = 13;


    // 输出渲染的贴图数量
    static constexpr unsigned FRAGOUT_TEXTURE_COUNT = 3;
    
    enum ETextureType
    {
        diffuse = 0,
        normal,
        roughness,
        metallic,
        ambient_occlusion,
        shadowmap,
    };

    /** 着色器类型（引擎侧整型），实现层映射到 GL_*_SHADER / VkShaderStageFlagBits */
    enum EShaderType : unsigned
    {
        vs  = 0,  // vertex
        fs  = 1,  // fragment
        gs  = 2,  // geometry
        cs  = 3,  // compute
        tcs = 4,  // tessellation control
        tes = 5,  // tessellation evaluation
    };
		
    class RenderMaterialInfo
    {
    public:
        RenderMaterialInfo() = default;
        virtual ~RenderMaterialInfo() = default;
        virtual void Initialize() {}
        glm::vec4 albedo {0.2f};
        float specular_factor {1.0f};
        float metallic {0.5f};
        float roughness {0.5f};
        float ao {0.3f};
        std::string name;

        void BindTextureByType(ETextureType textureType, unsigned int location);
        KongTexture* GetTextureByType(ETextureType textureType);

        virtual void AddMaterialByType(ETextureType textureType, std::weak_ptr<KongTexture> texture);
        std::map<ETextureType, std::weak_ptr<KongTexture>> textures;
    };

    struct Vertex
    {
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.f, 1.f, 0.f};
        glm::vec2 uv{0.0f};
        glm::vec3 tangent{0.0f};
        glm::vec3 bitangent{0.0f};
        /* Vulkan 顶点布局由实现层提供，见 VulkanPipeline.cpp 中 GetVertexBindingDescription / GetVertexAttributeDescription */
    };
    
    /** 渲染信息（与 API 无关句柄），实现层将 instance_buffer 转为 GLuint/VkBuffer */
    class RenderInfo
    {
    public:
        uint32_t instance_buffer = 0;
        unsigned instance_count = 0;
        
        virtual void Draw(void* commandBuffer) {}
        virtual void InitRenderInfo(){}

        template <class T>
        std::shared_ptr<T> GetMaterial();
        
        // 顶点
        std::unique_ptr<KongBuffer> vertex_buffer {nullptr};
        std::unique_ptr<KongBuffer> index_buffer {nullptr};

        // todo: 后续都放到这个格式里面,也要改造opengl的buffer不要搞太多buffer
        std::vector<Vertex> vertices;
        std::vector<unsigned int> m_Index;

        
        // 材质
        std::shared_ptr<RenderMaterialInfo> material {};
    };


    class CMesh
    {
    public:
        CMesh();
        
        std::unique_ptr<RenderInfo> m_RenderInfo {nullptr};
        std::string name;
    };


    // 渲染资源描述
    struct SRenderResourceDesc
    {
        // shader类型
        std::string shader_type;
        // 没有shader类型那就直接读取shader文件路径，尽量不要用这个
        std::map<EShaderType, std::string> shader_paths;
        std::map<ETextureType, std::string> texture_paths;

        std::string model_path;
        bool bOverloadMaterial = false;
        RenderMaterialInfo material;
    };

    struct SSceneLightInfo
    {
        // 场景光源信息
        std::weak_ptr<CDirectionalLightComponent> scene_dirlight;
        std::vector<std::weak_ptr<CPointLightComponent>> scene_pointlights;
        
        void clear()
        {
            scene_pointlights.clear();
        }
    };
    
    struct DirectionalLight
    {
        glm::vec4 light_dir;
        glm::vec4 light_color;
        glm::mat4 light_space_mat;
    };

    struct PointLight
    {
        glm::vec4 light_pos;
        glm::vec4 light_color;
    };

    // 先全部按照vec4对齐，用int和float等等算数据对齐还有问题需要后续解决
    constexpr unsigned POINT_LIGHT_MAX = 512;
    constexpr unsigned POINT_LIGHT_SHADOW_MAX = 4;
    struct SceneLightInfo
    {
        // todo: has_dir_light和point light count试着用int
        glm::ivec4 has_dir_light = glm::ivec4(0);
        DirectionalLight directional_light;
        glm::ivec4 point_light_count = glm::ivec4(0);
        PointLight point_lights[POINT_LIGHT_MAX];
        //只允许有四个点光源的阴影贴图，这里用ivec4传入对应点光源的index
        glm::ivec4 point_light_shadow_index = glm::ivec4(-1); 
    };
}
