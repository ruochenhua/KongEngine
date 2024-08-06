#pragma once
#include <vector>

#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace tinyGL
{
    class CPointLightComponent;
    class CDirectionalLightComponent;

    enum EShaderType
    {
        vs = GL_VERTEX_SHADER,		// vertex shader
        fs = GL_FRAGMENT_SHADER,	// fragment shader
        gs = GL_GEOMETRY_SHADER,	// geometry shader
    };
		
    struct SMaterial
    {
        glm::vec3 albedo = glm::vec3(0.f); 	
        float metallic = 0.5f;
        float roughness = 0.5;
        float ao = 0.3f;
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
		
        SMaterial material;
        // shader program
        unsigned vertex_size = 0;
        unsigned stride_count = 1;
        unsigned indices_count = 0;
        // texture id
        // todo: 总不能一个一个加吧，要支持类型映射
        GLuint diffuse_tex_id = 0;
        GLuint specular_tex_id = 0;
        GLuint normal_tex_id = 0;
        GLuint tangent_tex_id = 0;
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
		
        map<EShaderType, string> shader_paths;
        map<ETextureType, string> texture_paths;

        string model_path;
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
}
