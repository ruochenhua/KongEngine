#pragma once
#include "Common.h"

namespace Kong
{
    class Utils
    {
    public:
        static string ReadFile(const string& file_path)
        {
            std::string file_content;
            std::ifstream file_stream(file_path, std::ios::in);
            if (file_stream.is_open()) {
                std::stringstream sstr;
                sstr << file_stream.rdbuf();
                file_content = sstr.str();
                file_stream.close();
            }
            else
            {
                printf("Impossible to open %s. Are you in the right directory ? \n", file_path.c_str());
            }

            return file_content;
        }
    };

}
