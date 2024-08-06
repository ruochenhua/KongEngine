#pragma once
#include "Common.h"
#include "Engine.h"
#include "RenderCommon.h"

namespace tinyGL
{
	class AActor;

		
	// shader的主类型，后面每个shader的类型都会建一个类集成Shader类
	// 每个shader子类对应相同的shader编译文件，所以按理来说是可以每个类型的shader加载一次就行，而不需要每个模型加载一次，性能得以优化
    class Shader
    {
    public:
    	Shader() = default;
    	Shader(const SRenderResourceDesc& render_resource_desc);
	    virtual ~Shader() = default;
	    //static GLuint LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path)
    	static GLuint LoadShaders(const map<EShaderType, string>& shader_path_map)
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

    	void SetBool(const std::string &name, bool value)
	    {         
	        glUniform1i(glGetUniformLocation(shader_id, name.c_str()), (int)value); 
	    }
	    // ------------------------------------------------------------------------
	    void SetInt(const std::string &name, int value)
	    { 
	        glUniform1i(glGetUniformLocation(shader_id, name.c_str()), value); 
	    }
	    // ------------------------------------------------------------------------
	    void SetFloat(const std::string &name, float value)
	    { 
	        glUniform1f(glGetUniformLocation(shader_id, name.c_str()), value); 
	    }
	    // ------------------------------------------------------------------------
	    void SetVec2(const std::string &name, const glm::vec2 &value)
	    { 
	        glUniform2fv(glGetUniformLocation(shader_id, name.c_str()), 1, &value[0]); 
	    }
	    void SetVec2(const std::string &name, float x, float y)
	    { 
	        glUniform2f(glGetUniformLocation(shader_id, name.c_str()), x, y); 
	    }
	    // ------------------------------------------------------------------------
	    void SetVec3(const std::string &name, const glm::vec3 &value)
	    { 
	        glUniform3fv(glGetUniformLocation(shader_id, name.c_str()), 1, &value[0]); 
	    }
	    void SetVec3(const std::string &name, float x, float y, float z)
	    { 
	        glUniform3f(glGetUniformLocation(shader_id, name.c_str()), x, y, z); 
	    }
	    // ------------------------------------------------------------------------
	    void SetVec4(const std::string &name, const glm::vec4 &value)
	    { 
	        glUniform4fv(glGetUniformLocation(shader_id, name.c_str()), 1, &value[0]); 
	    }
	    void SetVec4(const std::string &name, float x, float y, float z, float w)
	    { 
	        glUniform4f(glGetUniformLocation(shader_id, name.c_str()), x, y, z, w); 
	    }
	    // ------------------------------------------------------------------------
	    void SetMat2(const std::string &name, const glm::mat2 &mat) 
	    {
	        glUniformMatrix2fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	    }
	    // ------------------------------------------------------------------------
	    void SetMat3(const std::string &name, const glm::mat3 &mat)
	    {
	        glUniformMatrix3fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	    }
	    // ------------------------------------------------------------------------
	    void SetMat4(const std::string &name, const glm::mat4 &mat)
	    {
	        glUniformMatrix4fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	    }

    	void Init(const map<EShaderType, string>& shader_path_cache);
    	// 使用该shader
    	void Use() const;
    	// 获取这个shader需要的数据，每个shader的需求应该是不一样的所以子类需要实现;
    	// 可能要从actor获取transform data，或者light里面获取light color之类的，所以先设定是要传入actor的指针
    	virtual void SetupData(CMesh& mesh) {};
    	virtual void UpdateRenderData(const CMesh& mesh,
    		const glm::mat4& actor_model_mat,
    		const SSceneRenderInfo& scene_render_info)
    	{};
    protected:
    	// shader id
    	GLuint shader_id = GL_NONE;
		// shader file path
    	map<EShaderType, string> shader_path_map;
    };
}
