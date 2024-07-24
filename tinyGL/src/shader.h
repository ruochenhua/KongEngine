#pragma once
#include "common.h"
#include "Engine.h"

// shader utility class
namespace tinyGL
{
    class Shader
    {
    public:
    	//static GLuint LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path)
    	static GLuint LoadShaders(const map<SRenderResourceDesc::EShaderType, string>& shader_path_map)
        {
    		vector<GLuint> shader_id_list;
    		for(auto& shader_path_pair : shader_path_map)
    		{
    			auto shader_type = shader_path_pair.first;
    			auto shader_path = shader_path_pair.second;

    			// EShaderType对照GL_XXXX_SHADER
    			GLuint shader_id = glCreateShader(shader_type);

    			// Read the Shader code from the file
    			std::string shader_code = Engine::ReadFile(shader_path);

    			GLint result = GL_FALSE;
    			int info_log_length;

    			// Compile Shader
    			printf("Compiling shader : %s\n", shader_path.c_str());
    			char const * shader_string_ptr = shader_code.c_str();
    			glShaderSource(shader_id, 1, &shader_string_ptr, NULL);
    			glCompileShader(shader_id);

    			// Check Shader
    			glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    			glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    			if (info_log_length > 0) {
    				std::vector<char> error_msg(info_log_length + 1);
    				glGetShaderInfoLog(shader_id, info_log_length, NULL, &error_msg[0]);
    				printf("%s\n", &error_msg[0]);
    				assert(0);
    			}
				shader_id_list.push_back(shader_id);
    		}
   
			// Link the program
			printf("Linking program\n");
			GLuint prog_id = glCreateProgram();
    		for(auto shader_id : shader_id_list)
    		{
    			glAttachShader(prog_id, shader_id);
    		}
			glLinkProgram(prog_id);

    		GLint result = GL_FALSE;
    		int info_log_length;
			// Check the program
			glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
			glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
			if (info_log_length > 0) {
				std::vector<char> prog_error_msg(info_log_length + 1);
				glGetProgramInfoLog(prog_id, info_log_length, NULL, &prog_error_msg[0]);
				printf("%s\n", &prog_error_msg[0]);
				assert(0);
			}

    		for(auto shader_id : shader_id_list)
    		{
    			glDetachShader(prog_id, shader_id);
				glDeleteShader(shader_id);
    		}

			return prog_id;
        }

    	static void SetBool(GLuint ID, const std::string &name, bool value)
	    {         
	        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
	    }
	    // ------------------------------------------------------------------------
	    static void SetInt(GLuint ID, const std::string &name, int value)
	    { 
	        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
	    }
	    // ------------------------------------------------------------------------
	    static void SetFloat(GLuint ID, const std::string &name, float value)
	    { 
	        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
	    }
	    // ------------------------------------------------------------------------
	    static void SetVec2(GLuint ID, const std::string &name, const glm::vec2 &value)
	    { 
	        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
	    }
	    static void SetVec2(GLuint ID, const std::string &name, float x, float y)
	    { 
	        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
	    }
	    // ------------------------------------------------------------------------
	    static void SetVec3(GLuint ID, const std::string &name, const glm::vec3 &value)
	    { 
	        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
	    }
	    static void SetVec3(GLuint ID, const std::string &name, float x, float y, float z)
	    { 
	        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
	    }
	    // ------------------------------------------------------------------------
	    static void SetVec4(GLuint ID, const std::string &name, const glm::vec4 &value)
	    { 
	        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
	    }
	    static void SetVec4(GLuint ID, const std::string &name, float x, float y, float z, float w)
	    { 
	        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
	    }
	    // ------------------------------------------------------------------------
	    static void SetMat2(GLuint ID, const std::string &name, const glm::mat2 &mat) 
	    {
	        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	    }
	    // ------------------------------------------------------------------------
	    static void SetMat3(GLuint ID, const std::string &name, const glm::mat3 &mat)
	    {
	        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	    }
	    // ------------------------------------------------------------------------
	    static void SetMat4(GLuint ID, const std::string &name, const glm::mat4 &mat)
	    {
	        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	    }
    };
}
