#pragma once
#include "common.h"
#include "Render/RenderCommon.hpp"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Kong
{
    // mesh的资源
    struct MeshResource
    {
        string directory;
        vector<shared_ptr<CMesh>> mesh_list;
    };

    // 资管管理类
    class ResourceManager
    {
    public:
        static shared_ptr<MeshResource> GetOrLoadMesh(const std::string & model_path);
        static GLuint GetOrLoadTexture(const std::string & texture_path, bool filp_uv = true);
        // 贴图
        GLuint GetTexture(const std::string & texture_path, bool flip_uv);
        
        static shared_ptr<KongTexture> GetOrLoadTexture_new(const std::string & texture_path, bool filp_uv = true);
        shared_ptr<KongTexture> GetTexture_new(const std::string & texture_path, bool flip_uv);
        
        static void Clean();
        
        // mesh
        shared_ptr<MeshResource> GetMesh(const std::string & mesh_path);

        void ProcessAssimpNode(const aiNode* model_node, const aiScene* scene, const shared_ptr<MeshResource>& mesh_resource);
        void ProcessAssimpMesh(aiMesh* mesh, const aiScene* scene, const shared_ptr<MeshResource>& mesh_resource);
    private:
        // 贴图储存
        map<string, GLuint> texture_cache;
        map<string, shared_ptr<KongTexture>> texture_cache_new;
        
        // 模型资源存储
        map<string, shared_ptr<MeshResource>> mesh_cache; 
    };
}