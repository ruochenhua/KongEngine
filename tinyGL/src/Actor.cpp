#include "Actor.h"

#include "MeshComponent.h"
#include "LightComponent.h"
#include "BoxShape.h"
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

void AActor::Update(float delta)
{
    // actor 更新调用
    // todo：先处理一下点光源和box mesh之间的颜色同步的问题吧，具体其他的还没想好
    auto point_light_comp = GetComponent<CPointLightComponent>();
    auto box_comp = GetComponent<CBoxShape>();
    if(point_light_comp && box_comp)
    {
        box_comp->mesh_list[0].m_RenderInfo.material.albedo = point_light_comp->light_color;
    }
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
    if(transform_comp)
    {
        return;
    }

    if(transform_comp->instancing_info.count > 0)
    {
        transform_comp->InitInstancingData();
        auto mesh_comp = GetComponent<CMeshComponent>();
        if(mesh_comp)
        {
            return;
        }

        transform_comp->BindInstancingToMesh(mesh_comp);
    }
}

