#pragma once
#include "common.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace tinyGL
{
	class SceneObject
	{
	public:
		glm::mat4 GetModelMatrix() const;
		glm::vec3 location = glm::vec3(0,0,0);
		glm::vec3 rotation = glm::vec3(0,0,0);
		glm::vec3 scale = glm::vec3(1,1,1);
	};

	class CMesh
	{
	public:		
		vector<float> GetVertices() const;
		vector<float> GetTextureCoords() const;
		vector<float> GetNormals() const;
		vector<unsigned int> GetIndices() const;
		vector<float> GetTangents() const;
		vector<float> GetBitangents() const;

		SRenderInfo GetRenderInfo() {return m_RenderInfo;}

		// virtual void GenerateRenderInfo() = 0;
		
		vector<glm::vec3> m_Vertex;
		vector<glm::vec3> m_Normal;
		vector<glm::vec2> m_TexCoord;
		vector<glm::vec3> m_Tangent;
		vector<glm::vec3> m_Bitangent;

		vector<unsigned int> m_Index;

		SRenderInfo m_RenderInfo;

		string directory;
	};
		
	class CRenderObj : public SceneObject
	{
	public:
		vector<CMesh> mesh_list;
		
		CRenderObj(const SRenderResourceDesc& render_resource_desc);	
		// virtual vector<float> GetVertices() const;
		// virtual vector<float> GetTextureCoords() const;
		// virtual vector<float> GetNormals() const;
		// virtual vector<unsigned int> GetIndices() const;
		// virtual vector<float> GetTangents() const;
		// virtual vector<float> GetBitangents() const;
		//
		// SRenderInfo GetRenderInfo() {return m_RenderInfo;}
		GLuint GetShaderId() const {return shader_id;}
	protected:
		virtual void GenerateRenderInfo() = 0;
		GLuint shader_id = GL_NONE;
		// vector<glm::vec3> m_Vertex;
		// vector<glm::vec3> m_Normal;
		// vector<glm::vec2> m_TexCoord;
		// vector<glm::vec3> m_Tangent;
		// vector<glm::vec3> m_Bitangent;
		//
		// vector<unsigned int> m_Index;
		//
		// SRenderInfo m_RenderInfo;

		string directory;
		
		// import obj model
		int ImportObj(const std::string& model_path);
		void ProcessAssimpNode(aiNode* model_node, const aiScene* scene);
		void ProcessAssimpMesh(aiMesh* mesh, const aiScene* scene);
	};
}
