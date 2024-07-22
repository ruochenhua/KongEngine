@@ -0,0 +1,99 @@
#pragma once
#include "common.h"

// shader utility class
namespace tinyGL
{
    class Shader
    {
    public:
    	static GLuint LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path)
        {
            // Create the shaders
			GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
			GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

			// Read the Vertex Shader code from the file
			std::string vs_code;
			std::ifstream vs_stream(vertex_file_path, std::ios::in);
			if (vs_stream.is_open()) {
				std::stringstream sstr;
				sstr << vs_stream.rdbuf();
				vs_code = sstr.str();
				vs_stream.close();
			}
			else {
				printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path.c_str());
				getchar();
				return 0;
			}

			// Read the Fragment Shader code from the file
			std::string fs_code;
			std::ifstream fs_stream(fragment_file_path, std::ios::in);
			if (fs_stream.is_open()) {
				std::stringstream sstr;
				sstr << fs_stream.rdbuf();
				fs_code = sstr.str();
				fs_stream.close();
			}

			GLint result = GL_FALSE;
			int info_log_length;

			// Compile Vertex Shader
			printf("Compiling shader : %s\n", vertex_file_path.c_str());
			char const * vs_ptr = vs_code.c_str();
			glShaderSource(vs_id, 1, &vs_ptr, NULL);
			glCompileShader(vs_id);

			// Check Vertex Shader
			glGetShaderiv(vs_id, GL_COMPILE_STATUS, &result);
			glGetShaderiv(vs_id, GL_INFO_LOG_LENGTH, &info_log_length);
			if (info_log_length > 0) {
				std::vector<char> vs_error_msg(info_log_length + 1);
				glGetShaderInfoLog(vs_id, info_log_length, NULL, &vs_error_msg[0]);
				printf("%s\n", &vs_error_msg[0]);
			}

			// Compile Fragment Shader
			printf("Compiling shader : %s\n", fragment_file_path.c_str());
			char const * fs_ptr = fs_code.c_str();
			glShaderSource(fs_id, 1, &fs_ptr, NULL);
			glCompileShader(fs_id);

			// Check Fragment Shader
			glGetShaderiv(fs_id, GL_COMPILE_STATUS, &result);
			glGetShaderiv(fs_id, GL_INFO_LOG_LENGTH, &info_log_length);
			if (info_log_length > 0) {
				std::vector<char> fs_error_msg(info_log_length + 1);
				glGetShaderInfoLog(fs_id, info_log_length, NULL, &fs_error_msg[0]);
				printf("%s\n", &fs_error_msg[0]);
			}

			// Link the program
			printf("Linking program\n");
			GLuint prog_id = glCreateProgram();
			glAttachShader(prog_id, vs_id);
			glAttachShader(prog_id, fs_id);
			glLinkProgram(prog_id);

			// Check the program
			glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
			glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
			if (info_log_length > 0) {
				std::vector<char> prog_error_msg(info_log_length + 1);
				glGetProgramInfoLog(prog_id, info_log_length, NULL, &prog_error_msg[0]);
				printf("%s\n", &prog_error_msg[0]);
			}

			glDetachShader(prog_id, vs_id);
			glDetachShader(prog_id, fs_id);

			glDeleteShader(vs_id);
			glDeleteShader(fs_id);

			return prog_id;
        }
    };
}
