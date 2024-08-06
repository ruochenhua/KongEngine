#include "Actor.h"

#include "MeshComponent.h"
#include "LightComponent.h"
using namespace tinyGL;

AActor::~AActor()
{
    for(auto& component : components)
    {
        component.reset();
    }
    
    components.clear();
    component_cache.clear();
}

void AActor::AddComponent(std::shared_ptr<CComponent> component)
{
    components.push_back(component);
}

glm::mat4 AActor::GetModelMatrix()
{
    // 赋值
    if(transform.expired())
    {
        transform = GetComponent<CTransformComponent>();
    }
    // 若没有transform组件，那就返回1，否则返回对于的模型矩阵
    if(!transform.expired())
    {
        return transform.lock()->GetModelMatrix();
    }

    return glm::mat4(1.0);
}

void AActor::BeginPlay()
{
    // 每个component也调用一下
    for(auto& component : components)
    {
        component->BeginPlay();
    }
    
    // 在加载完后需要做一些事情
    // 这里先处理一下instancing的东西，其他的后面再加
    auto transform_comp = GetComponent<CTransformComponent>();
    if(transform_comp.expired())
    {
        return;
    }

    if(transform_comp.lock()->instancing_info.count > 0)
    {
        transform_comp.lock()->InitInstancingData();
        auto mesh_comp = GetComponent<CMeshComponent>();
        if(mesh_comp.expired())
        {
            return;
        }

        transform_comp.lock()->BindInstancingToMesh(mesh_comp);
    }
}

