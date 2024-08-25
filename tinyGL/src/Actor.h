#pragma once
#include <memory>
#include <typeindex>

#include "common.h"

#include "Component/Mesh/MeshComponent.h"
namespace Kong
{
    class CComponent;
    struct InstancingInfo
    {
        unsigned count = 0;
        glm::vec3 location_min	= glm::vec3(0.0);
        glm::vec3 location_max	= glm::vec3(0.0);
        glm::vec3 rotation_min	= glm::vec3(0.0);
        glm::vec3 rotation_max	= glm::vec3(0.0);
        glm::vec3 scale_min		= glm::vec3(0.0);
        glm::vec3 scale_max		= glm::vec3(0.0);
    };
    // 参考UE的Actor
    class AActor
    {
    public:
        AActor() = default;
        ~AActor();
        
        void AddComponent(std::shared_ptr<CComponent> component);
        void Update(double delta);
        template<class T>
        shared_ptr<T> GetComponent();
        glm::mat4 GetModelMatrix() const;
        std::string name;

        void BeginPlay();
        
        glm::vec3 location	= glm::vec3(0,0,0);
        glm::vec3 rotation	= glm::vec3(0,0,0);
        glm::vec3 scale		= glm::vec3(1,1,1);
        InstancingInfo instancing_info;

        
        glm::mat4 GenInstanceModelMatrix() const;
        void InitInstancingData();
        void BindInstancingToMesh(weak_ptr<CMeshComponent> mesh_comp);
        const glm::mat4& GetInstancingModelMat(unsigned idx) const;

    protected:
        std::vector<std::shared_ptr<CComponent>> components;
        // 可能比较常用，可以缓存一下
        //std::shared_ptr<CTransformComponent> transform;

        map<std::type_index, std::weak_ptr<CComponent>> component_cache;

    
    private:
        std::vector<glm::mat4> instancing_model_mat;
    };

    template <class T>
    shared_ptr<T> AActor::GetComponent()
    {
        std::type_index type_index = typeid(T);
        auto cache_iter = component_cache.find(type_index); 
        if(cache_iter != component_cache.end())
        {
            return std::dynamic_pointer_cast<T>(cache_iter->second.lock());
        }
        
        for(auto component : components)
        {
            auto pointer = std::dynamic_pointer_cast<T>(component);
            if(pointer)
            {
                component_cache.emplace(type_index, pointer);
                return pointer;
            }
        }

        return nullptr;
    }
}
