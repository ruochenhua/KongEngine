#pragma once
#include "Common.h"
#include "Component.h"
#include "Shader/Shader.h"

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


		
	class CMeshComponent : public CComponent
	{
	public:
		vector<CMesh> mesh_list;
		shared_ptr<Shader> shader_data;
		
		CMeshComponent(const SRenderResourceDesc& render_resource_desc);	

		void BeginPlay() override;

	protected:
		virtual void GenerateRenderInfo() = 0;
		//GLuint shader_id = GL_NONE;

		
		string directory;
		
		// import obj model
		int ImportObj(const std::string& model_path);
		void ProcessAssimpNode(aiNode* model_node, const aiScene* scene);
		void ProcessAssimpMesh(aiMesh* mesh, const aiScene* scene);
	};
}
