#pragma once
#include "common.h"
#include "Render/RenderCommon.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Kong
{
    // mesh的资源
    struct MeshResource
    {
        string directory;
        vector<CMesh> mesh_list;
    };

    // 资管管理类
    class ResourceManager
    {
    public:
        static shared_ptr<MeshResource> GetOrLoadMesh(const std::string & model_path);
        static GLuint GetOrLoadTexture(const std::string & texture_path, bool filp_uv = true);

        // 贴图
        GLuint GetTexture(const std::string & texture_path, bool flip_uv);

        // mesh
        shared_ptr<MeshResource> GetMesh(const std::string & mesh_path);

        void ProcessAssimpNode(const aiNode* model_node, const aiScene* scene, const shared_ptr<MeshResource>& mesh_resource);
        void ProcessAssimpMesh(aiMesh* mesh, const aiScene* scene, const shared_ptr<MeshResource>& mesh_resource);
    private:
        // 贴图储存
        map<string, GLuint> texture_cache;
        // 模型资源存储
        map<string, shared_ptr<MeshResource>> mesh_cache; 
    };
}