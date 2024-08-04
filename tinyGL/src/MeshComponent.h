#pragma once
#include "Common.h"
#include "Component.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace tinyGL
{
	class CMeshComponent;
	struct InstancingInfo
	{
		unsigned count = 0;
		glm::vec3 location_min	= glm::vec3(0.0);
		glm::vec3 location_max	= glm::vec3(0.0);
		glm::vec3 rotation_min	= glm::vec3(0.0);
		glm::vec3 rotation_max	= glm::vec3(0.0);
		glm::vec3 scale_min		= glm::vec3(0.0);
		glm::vec3 scale_max		= glm::vec3(0.0);

		GLuint instance_buffer = GL_NONE;
	};
	
	class CTransformComponent : public CComponent
	{
	public:
		glm::mat4 GetModelMatrix() const;
		glm::mat4 GenInstanceModelMatrix() const;
		glm::vec3 location	= glm::vec3(0,0,0);
		glm::vec3 rotation	= glm::vec3(0,0,0);
		glm::vec3 scale		= glm::vec3(1,1,1);


		void InitInstancingData();
		void BindInstancingToMesh(weak_ptr<CMeshComponent> mesh_comp);
		
		InstancingInfo instancing_info;
		virtual const glm::mat4& GetInstancingModelMat(unsigned idx) const;

	private:
		std::vector<glm::mat4> instancing_model_mat;
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

		SRenderInfo GetRenderInfo() const { return m_RenderInfo; }

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
		
	class CMeshComponent : public CComponent
	{
	public:
		vector<CMesh> mesh_list;
		
		CMeshComponent(const SRenderResourceDesc& render_resource_desc);	
		GLuint GetShaderId() const {return shader_id;}
		

	protected:
		virtual void GenerateRenderInfo() = 0;
		GLuint shader_id = GL_NONE;

		string directory;
		
		// import obj model
		int ImportObj(const std::string& model_path);
		void ProcessAssimpNode(aiNode* model_node, const aiScene* scene);
		void ProcessAssimpMesh(aiMesh* mesh, const aiScene* scene);
	};
}
