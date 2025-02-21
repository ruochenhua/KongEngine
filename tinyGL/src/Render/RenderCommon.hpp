#pragma once
#include <vector>
#ifdef RENDER_IN_VULKAN
#include <vulkan/vulkan_core.h>
#endif

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "Resource/Buffer.hpp"

#define SHADOWMAP_DEBUG 0
#define USE_CSM 1


namespace Kong
{
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
    };

    enum EShaderType 
    {
        vs = GL_VERTEX_SHADER,		        // vertex shader
        fs = GL_FRAGMENT_SHADER,	        // fragment shader
        gs = GL_GEOMETRY_SHADER,	        // geometry shader
        cs = GL_COMPUTE_SHADER,
        tcs = GL_TESS_CONTROL_SHADER,       // tessellation control shader
        tes = GL_TESS_EVALUATION_SHADER,    // tessellation evaluation shader
    };
		
    struct SMaterialInfo
    {
        SMaterialInfo() = default;
        glm::vec4 albedo = glm::vec4(0.2f);
        float specular_factor = 1.0f;
        float metallic = 0.5f;
        float roughness = 0.5;
        float ao = 0.3f;
        std::string name;

        // texture id
        std::map<ETextureType, GLuint> textures;
        GLuint diffuse_tex_id = 0;
        GLuint normal_tex_id = 0;
        GLuint roughness_tex_id = 0;
        GLuint metallic_tex_id = 0;
        GLuint ao_tex_id = 0;
    };

    struct Vertex
    {
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.f, 1.f, 0.f};
        glm::vec2 uv{0.0f};
        glm::vec3 tangent{0.0f};
        glm::vec3 bitangent{0.0f};
        
#ifdef RENDER_IN_VULKAN
        static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
#endif
    };
    
    // 渲染信息 //todo: 改名, opengl和vulkan在这里做区分
    class SRenderInfo
    {
    public:
        GLuint instance_buffer = 0;
        unsigned instance_count = 0;
        
        virtual void Draw(void* commandBuffer) {}
        virtual void InitRenderInfo(){}
        
        // 顶点
        std::unique_ptr<KongBuffer> vertex_buffer {nullptr};
        std::unique_ptr<KongBuffer> index_buffer {nullptr};

        // todo: 后续都放到这个格式里面,也要改造opengl的buffer不要搞太多buffer
        std::vector<Vertex> vertices;
        std::vector<unsigned int> m_Index;
        
        // 材质
        SMaterialInfo material;
    };
    

    
    class CMesh
    {
    public:
        CMesh();
        
        std::unique_ptr<SRenderInfo> m_RenderInfo {nullptr};
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
        SMaterialInfo material;
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
        glm::ivec4 has_dir_light = glm::ivec4(0);
        DirectionalLight directional_light;
        glm::ivec4 point_light_count = glm::ivec4(0);
        PointLight point_lights[POINT_LIGHT_MAX];
        //只允许有四个点光源的阴影贴图，这里用ivec4传入对应点光源的index
        glm::ivec4 point_light_shadow_index = glm::ivec4(-1); 
    };
}
