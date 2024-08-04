#pragma once
#include <memory>
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

        void AddComponent(std::shared_ptr<CComponent> component);
        
        template<class T>
        weak_ptr<T> GetComponent();
        glm::mat4 GetModelMatrix();
        std::string name;

        void BeginPlay();
        
    protected:
        std::vector<std::shared_ptr<CComponent>> components;
        // 可能比较常用，可以缓存一下
        std::weak_ptr<CTransformComponent> transform;
        
    };

    template <class T>
    weak_ptr<T> AActor::GetComponent()
        {
            for(auto component : components)
            {
                auto pointer = std::dynamic_pointer_cast<T>(component);
                if(pointer)
                {
                    return pointer;
                }
            }

            return std::weak_ptr<T>();
        }

    
}
