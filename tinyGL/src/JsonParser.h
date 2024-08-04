#pragma once
#include <memory>
#include <string>
#include <vector>

namespace tinyGL
{
    class AActor;

    class CJsonParser
    {
    public:
        static void ParseJsonFile(const std::string& scene_content,
            std::vector<std::shared_ptr<AActor>>& scene_actors);
    };
}
