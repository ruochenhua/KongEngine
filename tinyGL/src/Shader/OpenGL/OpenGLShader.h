#pragma once

#include "Common.h"
#include "Render/RenderCommon.hpp"
#include <map>
#include <set>

namespace Kong
{
	class AActor;
	class ShaderManager;

	// shader的主类型，后面每个shader的类型都会建一个类集成Shader类
	// 每个shader子类对应相同的shader编译文件，所以按理来说是可以每个类型的shader加载一次就行，而不需要每个模型加载一次，性能得以优化
	class OpenGLShader
	{
	public:
		OpenGLShader() = default;
		OpenGLShader(const map<EShaderType, string>& shader_paths);
		virtual ~OpenGLShader() = default;
		static GLuint LoadShaders(const map<EShaderType, string>& shader_paths);
		/// 将 include 文件加入缓存（路径相对于 shader 根目录，可有可无前导 /）
		static void IncludeShader(const string& include_path);
		static std::vector<std::string> FindIncludeFiles(const string& code_content);
		/// C++ 端预处理：展开 #include 并插入 #line，移除对 GL_ARB_shading_language_include 的依赖
		static std::string PreProcessShader(const std::string& source, const std::string& current_file, int start_line, std::set<std::string>& visiting);

	private:
		/// include 路径 -> 预处理后的内容缓存
		static std::map<std::string, std::string> s_include_cache;
		static void LoadIncludeToCache(const std::string& normalized_path, std::set<std::string>& visiting);
		static std::string NormalizeIncludePath(const std::string& path);
		static std::string ResolveIncludePath(const std::string& normalized_path);
		static int CountNewlines(std::string::const_iterator beg, std::string::const_iterator fin);
		static bool IsIncludeLineCommented(const std::string& code, std::string::const_iterator includeStart);
#if USE_DSA
	public:
    	void SetBool(const std::string &name, bool value)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform1i(shader_id, location, value);
	    }
	    // ------------------------------------------------------------------------
	    void SetInt(const std::string &name, int value)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform1i(shader_id, location, value);
	    }
	    // ------------------------------------------------------------------------
	    void SetFloat(const std::string &name, float value)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform1f(shader_id, location, value);
	    }

    	// ------------------------------------------------------------------------
    	void SetDouble(const std::string &name, double value)
    	{
    		GLint location = GetVariableLocation(name);
    		glProgramUniform1d(shader_id, location, value);
    	}
    	
	    // ------------------------------------------------------------------------
	    void SetVec2(const std::string &name, const glm::vec2 &value)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform2fv(shader_id, location, 1, &value[0]);
	    }
	    void SetVec2(const std::string &name, float x, float y)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform2f(shader_id, location, x, y);
	    }
	    // ------------------------------------------------------------------------
	    void SetVec3(const std::string &name, const glm::vec3 &value)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform3fv(shader_id, location, 1, &value[0]);
	    }
	    void SetVec3(const std::string &name, float x, float y, float z)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform3f(shader_id, location, x, y, z);
	    }
	    // ------------------------------------------------------------------------
	    void SetVec4(const std::string &name, const glm::vec4 &value)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform4fv(shader_id, location, 1, &value[0]);
	    }
	    void SetVec4(const std::string &name, float x, float y, float z, float w)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniform4f(shader_id, location, x, y, z, w);
	    }
	    // ------------------------------------------------------------------------
	    void SetMat2(const std::string &name, const glm::mat2 &mat) 
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniformMatrix2fv(shader_id, location, 1, GL_FALSE, &mat[0][0]);
	    }
	    // ------------------------------------------------------------------------
	    void SetMat3(const std::string &name, const glm::mat3 &mat)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniformMatrix3fv(shader_id, location, 1, GL_FALSE, &mat[0][0]);
	    }
	    // ------------------------------------------------------------------------
	    void SetMat4(const std::string &name, const glm::mat4 &mat)
	    {
    		GLint location = GetVariableLocation(name);
    		glProgramUniformMatrix4fv(shader_id, location, 1, GL_FALSE, &mat[0][0]);
	    }
#else
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
    	void SetDouble(const std::string &name, double value)
    	{ 
    		glUniform1d(glGetUniformLocation(shader_id, name.c_str()), value); 
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
#endif
    	void Init(const map<EShaderType, string>& shader_path_cache);
    	// 使用该shader
    	void Use() const;
    	// 获取这个shader需要的数据，每个shader的需求应该是不一样的所以子类需要实现;
    	// 父类的这个是为了支持原先的传入shader文件的写法，也就是设置尽量全名的参数传入
    	virtual void UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material);
    	
    	bool bIsBlend = false;
    	
    protected:
    	// shader id
    	GLuint shader_id = GL_NONE;
		// shader file path
    	map<EShaderType, string> shader_path_map;
		map<string, GLint> variable_location_map_;

    	GLint GetVariableLocation(const string& variable_name);
    };
	
	class ShaderManager
	{
	public:
		static shared_ptr<OpenGLShader> GetShader(const string& shader_name);

	protected:
		shared_ptr<OpenGLShader> GetShaderFromTypeName(const string& shader_name);
	private:
		
		std::map<string, shared_ptr<OpenGLShader>> shader_cache;
	};
}
