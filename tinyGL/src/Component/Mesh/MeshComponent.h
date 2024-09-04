#pragma once
#include "Common.h"
#include "Component/Component.h"
#include "Parser/ResourceManager.h"
#include "Shader/Shader.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Kong
{
	class CMeshComponent;
	
	// class CTransformComponent : public CComponent
	// {
	// public:
	// 	glm::mat4 GetModelMatrix() const;
	// 	glm::mat4 GenInstanceModelMatrix() const;
	// 	glm::vec3 location	= glm::vec3(0,0,0);
	// 	glm::vec3 rotation	= glm::vec3(0,0,0);
	// 	glm::vec3 scale		= glm::vec3(1,1,1);
	//
	//
	// 	void InitInstancingData();
	// 	void BindInstancingToMesh(weak_ptr<CMeshComponent> mesh_comp);
	// 	
	// 	InstancingInfo instancing_info;
	// 	virtual const glm::mat4& GetInstancingModelMat(unsigned idx) const;
	//
	// private:
	// 	std::vector<glm::mat4> instancing_model_mat;
	// };
	
	class CMeshComponent : public CComponent
	{
	public:
		shared_ptr<MeshResource> mesh_resource;
		shared_ptr<Shader> shader_data;
		
		CMeshComponent(const SRenderResourceDesc& render_resource_desc);	
		
		void BeginPlay() override;
		virtual void Draw(const SSceneRenderInfo& scene_render_info);
		virtual void InitRenderInfo();
		bool IsBlend();

		// 覆盖原有材质 
		SRenderInfo override_render_info;
		bool use_override_material = false;
	protected:
		// import obj model
		// todo：同样的资源复用
		int ImportObj(const std::string& model_path);
		void LoadOverloadTexture(const SRenderResourceDesc& render_resource_desc);
	};
}
