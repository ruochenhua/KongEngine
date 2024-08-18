#pragma once
#include <vector>

#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace tinyGL
{
    class CPointLightComponent;
    class CDirectionalLightComponent;

    constexpr unsigned DIFFUSE_TEX_SHADER_ID = 0;
    constexpr unsigned NORMAL_TEX_SHADER_ID = 1;
    constexpr unsigned ROUGHNESS_TEX_SHADER_ID = 2;
    constexpr unsigned METALLIC_TEX_SHADER_ID = 3;
    constexpr unsigned AO_TEX_SHADER_ID = 4;
    constexpr unsigned SKYBOX_TEX_SHADER_ID = 5;
    constexpr unsigned DIRLIGHT_SM_TEX_SHADER_ID = 6;
    constexpr unsigned POINTLIGHT_SM_TEX_SHADER_ID = 7;
    
    enum EShaderType
    {
        vs = GL_VERTEX_SHADER,		// vertex shader
        fs = GL_FRAGMENT_SHADER,	// fragment shader
        gs = GL_GEOMETRY_SHADER,	// geometry shader
    };
		
    struct SMaterial
    {
        glm::vec4 albedo = glm::vec4(0.f);
        float specular_factor = 1.0f;
        float metallic = 0.5f;
        float roughness = 0.5;
        float ao = 0.3f;
        string name;
    };

    // 渲染信息
    struct SRenderInfo
    {
        // vertex buffer id(vbo)
        GLuint vertex_buffer = 0;
        // ibo
        GLuint index_buffer = 0;
        // vao
        GLuint vertex_array_id = 0;
        GLuint texture_buffer = 0;
        GLuint normal_buffer = 0;
        GLuint tangent_buffer = 0;
        GLuint bitangent_buffer = 0;
        GLuint instance_buffer = 0;
		
        // shader program
        unsigned vertex_size = 0;
        unsigned stride_count = 1;
        unsigned indices_count = 0;
        unsigned instance_count = 0;

        // texture id
        GLuint diffuse_tex_id = 0;
        GLuint specular_tex_id = 0;
        GLuint normal_tex_id = 0;
        GLuint tangent_tex_id = 0;
        GLuint roughness_tex_id = 0;
        GLuint metallic_tex_id = 0;
        GLuint ao_tex_id = 0;
        
        // 材质
        SMaterial material;
    };
	
    class CMesh
    {
    public:		
        std::vector<float> GetVertices() const;
        std::vector<float> GetTextureCoords() const;
        std::vector<float> GetNormals() const;
        std::vector<unsigned int> GetIndices() const;
        std::vector<float> GetTangents() const;
        std::vector<float> GetBitangents() const;

        SRenderInfo GetRenderInfo() const { return m_RenderInfo; }

        // virtual void GenerateRenderInfo() = 0;
		
        std::vector<float> m_Vertex;
        std::vector<float> m_Normal;
        std::vector<float> m_TexCoord;
        std::vector<float> m_Tangent;
        std::vector<float> m_Bitangent;

        std::vector<unsigned int> m_Index;

        SRenderInfo m_RenderInfo;
        string name;
    };

    // 渲染资源描述
    struct SRenderResourceDesc
    {
        enum ETextureType
        {
            diffuse = 0,
            specular,
            normal,
            tangent,
            metallic,
            roughness,
            ambient_occlusion,
            glow,
        };

        // shader类型
        string shader_type;
        // 没有shader类型那就直接读取shader文件路径，尽量不要用这个
        map<EShaderType, string> shader_paths;
        map<ETextureType, string> texture_paths;

        string model_path;
        bool bOverloadMaterial = false;
        SMaterial material;
    };

    struct SSceneRenderInfo
    {
        glm::vec3 camera_pos;
        glm::mat4 camera_view;
        glm::mat4 camera_proj;
        
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
    constexpr unsigned POINT_LIGHT_MAX = 4;
    struct SceneLightInfo
    {
        glm::ivec4 has_dir_light = glm::ivec4(0);;
        DirectionalLight directional_light;
        glm::ivec4 point_light_count = glm::ivec4(0);
        PointLight point_lights[POINT_LIGHT_MAX];
   
    };

}
