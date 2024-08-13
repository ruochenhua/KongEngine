#pragma once
#include <memory>
#include <typeindex>

#include "common.h"

#include "LightComponent.h"
#include "MeshComponent.h"
namespace tinyGL
{
    class CTransformComponent;
    class CComponent;

    // 参考UE的Actor
    class AActor
    {
    public:
        AActor() = default;
        ~AActor();
        
        void AddComponent(std::shared_ptr<CComponent> component);
        void Update(float delta);
        
        template<class T>
        shared_ptr<T> GetComponent();
        glm::mat4 GetModelMatrix();
        std::string name;

        void BeginPlay();
        
    protected:
        std::vector<std::shared_ptr<CComponent>> components;
        // 可能比较常用，可以缓存一下
        std::weak_ptr<CTransformComponent> transform;

        map<std::type_index, std::weak_ptr<CComponent>> component_cache;
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
