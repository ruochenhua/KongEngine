#pragma once

namespace Kong
{
    // 参考UE的Actor
    class CComponent
    {
    public:
        virtual ~CComponent() = default;
        virtual void BeginPlay() {}
    };
    
}
