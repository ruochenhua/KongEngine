#pragma once
#include "Common.h"

namespace Kong
{
    class Utils
    {
    public:
        static string ReadFile(const string& file_path)
        {
            // std::string file_content;
            // std::ifstream file_stream(file_path, std::ios::in);
            // if (file_stream.is_open()) {
            //     std::stringstream sstr;
            //     sstr << file_stream.rdbuf();
            //     file_content = sstr.str();
            //     file_stream.close();
            // }
            // else
            // {
            //     printf("Impossible to open %s. Are you in the right directory ? \n", file_path.c_str());
            // }
            //
            // return file_content;

            ifstream file(file_path, std::ios::binary | std::ios::ate);
            if (!file.is_open())
            {
                throw std::runtime_error("Failed to open file: " + file_path);
            }

            size_t fileSize = file.tellg();
            vector<char> data(fileSize);

            file.seekg(0);
            file.read(data.data(), fileSize);

            file.close();

            return string(data.data(), data.size());
        }
    };

}
