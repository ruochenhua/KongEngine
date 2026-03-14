#include "OpenGLShader.h"

#include <cassert>
#include <cctype>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "BlendShader.h"
#include "DeferInfoShader.h"
#include "EmitShader.h"
#include "Utils.hpp"
#include "Render/RenderModule.hpp"
#include "Scene.hpp"
#include "ShadowMapShader.h"

using namespace Kong;
using namespace glm;

ShaderManager* g_shader_manager = new ShaderManager;
set<string> shader_include_set;

std::map<std::string, std::string> OpenGLShader::s_include_cache;

std::string OpenGLShader::NormalizeIncludePath(const std::string& path) {
	std::string p = path;
	while (!p.empty() && (p.front() == ' ' || p.front() == '\t')) p.erase(0, 1);
	while (!p.empty() && (p.back() == ' ' || p.back() == '\t')) p.pop_back();
	if (!p.empty() && p.front() == '/') p.erase(0, 1);
	return p;
}

std::string OpenGLShader::ResolveIncludePath(const std::string& normalized_path) {
	return CSceneLoader::ToResourcePath("shader/" + normalized_path);
}

int OpenGLShader::CountNewlines(std::string::const_iterator beg, std::string::const_iterator fin) {
	int n = 0;
	for (; beg != fin; ++beg) if (*beg == '\n') ++n;
	return n;
}

bool OpenGLShader::IsIncludeLineCommented(const std::string& code, std::string::const_iterator includeStart) {
	std::string::const_iterator p = includeStart;
	while (p != code.cbegin() && *(p - 1) != '\n') --p;
	while (p != includeStart && (*p == ' ' || *p == '\t')) ++p;
	return (includeStart - p >= 2 && *p == '/' && *(p + 1) == '/');
}

OpenGLShader::OpenGLShader(const map<EShaderType, string>& shader_paths)
{
	shader_path_map = shader_paths;
    Init(shader_paths);
}

GLuint OpenGLShader::LoadShaders(const map<EShaderType, string>& shader_paths)
{
    vector<GLuint> shader_id_list;
    for(auto& shader_path_pair : shader_paths)
    {
    	EShaderType shader_type = shader_path_pair.first;
    	auto shader_path = shader_path_pair.second;

    	// EShaderType 映射到 GL_*_SHADER（引擎枚举 0,1,2... 不能直接传给 glCreateShader）
    	static const GLenum kShaderTypeToGL[] = {
    		GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER,
    		GL_COMPUTE_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER
    	};
    	unsigned st = static_cast<unsigned>(shader_type);
    	GLenum glType = (st < sizeof(kShaderTypeToGL) / sizeof(kShaderTypeToGL[0]))
    		? kShaderTypeToGL[st] : GL_VERTEX_SHADER;
    	GLuint shader_id = glCreateShader(glType);

    	std::string shader_code = Utils::ReadFile(shader_path);
    	std::set<std::string> visiting;
    	std::string processed = PreProcessShader(shader_code, shader_path, 1, visiting);

    	GLint result = GL_FALSE;
    	int info_log_length;

    	printf("Compiling shader : %s\n", shader_path.c_str());
    	char const* shader_string_ptr = processed.c_str();
    	glShaderSource(shader_id, 1, &shader_string_ptr, NULL);
    	glCompileShader(shader_id);

    	// Check Shader
    	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    	if (info_log_length > 0) {
    		std::vector<char> error_msg(info_log_length + 1);
    		glGetShaderInfoLog(shader_id, info_log_length, NULL, &error_msg[0]);
    		printf("%s\n", &error_msg[0]);
    		assert(0 && "Shader load failed");
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
		assert(0 && "Program link failed");
	}

    for(auto shader_id : shader_id_list)
    {
    	glDetachShader(prog_id, shader_id);
		glDeleteShader(shader_id);
    }
	
	return prog_id;
}

void OpenGLShader::IncludeShader(const string& include_path)
{
	std::string key = NormalizeIncludePath(include_path);
	if (s_include_cache.find(key) != s_include_cache.end())
		return;
	std::set<std::string> visiting;
	std::string full_path = ResolveIncludePath(key);
	std::string content;
	try {
		content = Utils::ReadFile(full_path);
	} catch (const std::exception& e) {
		fprintf(stderr, "[Shader] Include not found: %s (resolved: %s)\n", key.c_str(), full_path.c_str());
		throw;
	}
	visiting.insert(key);
	s_include_cache[key] = PreProcessShader(content, key, 1, visiting);
}

void OpenGLShader::LoadIncludeToCache(const std::string& normalized_path, std::set<std::string>& visiting)
{
	if (s_include_cache.count(normalized_path))
		return;
	if (visiting.count(normalized_path)) {
		fprintf(stderr, "[Shader] Cycle detected in #include: %s\n", normalized_path.c_str());
		throw std::runtime_error("Shader include cycle detected: " + normalized_path);
	}
	visiting.insert(normalized_path);
	std::string full_path = ResolveIncludePath(normalized_path);
	std::string content;
	try {
		content = Utils::ReadFile(full_path);
	} catch (const std::exception& e) {
		fprintf(stderr, "[Shader] Include file not found: %s (resolved: %s)\n", normalized_path.c_str(), full_path.c_str());
		visiting.erase(normalized_path);
		throw;
	}
	std::string processed = PreProcessShader(content, normalized_path, 1, visiting);
	s_include_cache[normalized_path] = processed;
	visiting.erase(normalized_path);
}

// #include "path" 或 #include <path>（用自定义分隔符避免 )" 提前结束 raw 字符串）
static const std::regex s_include_regex(R"re(#\s*include\s*(?:"([^"]*)"|<([^>]*)>))re");
// 整行 #extension GL_ARB_shading_language_include : require
static const std::regex s_extension_include_regex(
	R"re([ \t]*#\s*extension\s+GL_ARB_shading_language_include\s*:\s*require[ \t]*(?:\n|$))re",
	std::regex::icase);

std::string OpenGLShader::PreProcessShader(const std::string& source, const std::string& current_file, int start_line, std::set<std::string>& visiting)
{
	std::string result = source;
	const size_t kMaxIncludeExpansions = 512u;
	size_t expansion_count = 0;
	// 1) 展开 #include（仅处理行首的 #include，避免匹配到字符串/注释中的字面量导致死循环）
	while (expansion_count < kMaxIncludeExpansions) {
		std::smatch m;
		if (!std::regex_search(result, m, s_include_regex))
			break;
		size_t pos = m.position(0);
		size_t line_start = result.rfind('\n', pos);
		line_start = (line_start == std::string::npos) ? 0 : line_start + 1;
		// 只展开“行首（仅空白）+ #include”，否则可能是字符串里的 "#include \"...\"" 会反复匹配导致死循环
		size_t p = line_start;
		while (p < pos && p < result.size() && (result[p] == ' ' || result[p] == '\t')) ++p;
		if (p != pos) {
			// 行首到 #include 之间非空白，当作非指令跳过（避免死循环）
			result.replace(pos, 1u, " "); // 破坏该处 "#" 避免再次匹配
			continue;
		}
		// 跳过行首空白后检查是否被 // 注释
		p = line_start;
		while (p < result.size() && (result[p] == ' ' || result[p] == '\t')) ++p;
		bool commented = (p + 2 <= result.size() && result[p] == '/' && result[p + 1] == '/');
		std::string path = m.str(1).empty() ? m.str(2) : m.str(1);
		if (commented) {
			size_t line_end = result.find('\n', line_start);
			line_end = (line_end == std::string::npos) ? result.size() : line_end + 1;
			result.erase(line_start, line_end - line_start);
			continue;
		}
		std::string normalized = NormalizeIncludePath(path);
		LoadIncludeToCache(normalized, visiting);
		const std::string& included = s_include_cache[normalized];
		int line_no = start_line + static_cast<int>(std::count(result.cbegin(), result.cbegin() + pos, '\n'));
		std::string replacement = "\n#line 1 0\n" + included + "\n#line " + std::to_string(line_no + 1) + " 0\n";
		size_t line_end = result.find('\n', pos);
		line_end = (line_end == std::string::npos) ? result.size() : line_end + 1;
		result.replace(line_start, line_end - line_start, replacement);
		++expansion_count;
	}
	// 2) 去掉 #extension GL_ARB_shading_language_include : require 整行
	result = std::regex_replace(result, s_extension_include_regex, "");
	return result;
}

std::vector<std::string> OpenGLShader::FindIncludeFiles(const string& code_content)
{
	std::vector<std::string> includes;
	std::istringstream iss(code_content);
	std::string line;
	while (std::getline(iss, line)) {
		size_t j = 0;
		while (j < line.size() && (line[j] == ' ' || line[j] == '\t')) ++j;
		if (j + 9 >= line.size()) continue;
		if (line.compare(j, 9, "#include ") != 0) continue;
		j += 9;
		while (j < line.size() && (line[j] == ' ' || line[j] == '\t')) ++j;
		if (j >= line.size() || (line[j] != '"' && line[j] != '<')) continue;
		char close_ch = (line[j] == '"') ? '"' : '>';
		++j;
		size_t path_start = j;
		size_t path_end = line.find(close_ch, j);
		if (path_end == std::string::npos) continue;
		includes.push_back(line.substr(path_start, path_end - path_start));
	}
	return includes;
}


void OpenGLShader::Init(const map<EShaderType, string>& shader_path_cache)
{
    shader_id = OpenGLShader::LoadShaders(shader_path_cache);
	assert(shader_id && "Shader load failed");
}

void OpenGLShader::Use() const
{
	assert(shader_id && "Shader not loaded yet");
	glUseProgram(shader_id);
}

void OpenGLShader::UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material)
{
	// 材质属性
	SetVec4("albedo", render_material->albedo);
	SetFloat("specular_factor", render_material->specular_factor);
	SetFloat("metallic", render_material->metallic);
	SetFloat("roughness", render_material->roughness);
	SetFloat("ao", render_material->ao);

	/*
	法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
	Normal = mat3(transpose(inverse(model))) * aNormal;
	 */
	render_material->BindTextureByType(diffuse, 0);
	render_material->BindTextureByType(normal, 1);
}

GLint OpenGLShader::GetVariableLocation(const string& variable_name)
{
	if (variable_location_map_.find(variable_name) != variable_location_map_.end())
	{
		return variable_location_map_[variable_name];
	}

	GLint location = glGetUniformLocation(shader_id, variable_name.c_str());
	variable_location_map_.emplace(variable_name, location);
	return location;
}

shared_ptr<OpenGLShader> ShaderManager::GetShader(const string& shader_name)
{
	return g_shader_manager->GetShaderFromTypeName(shader_name);
}

shared_ptr<OpenGLShader> ShaderManager::GetShaderFromTypeName(const string& shader_name)
{
	auto find_iter = shader_cache.find(shader_name);
	if(find_iter != shader_cache.end())
	{
		return find_iter->second;
	}

	// create shader
	if(shader_name == "brdf")
	{
		auto shader_data = make_shared<DeferInfoShader>();

		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "emit")
	{
		auto shader_data = make_shared<EmitShader>();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "blend")
	{
		auto shader_data = make_shared<BlendShader>();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "point_light_shadowmap")
	{
		auto shader_data = make_shared<PointLightShadowMapShader>();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "directional_light_shadowmap")
	{
#if USE_CSM
		auto shader_data = make_shared<DirectionalLightCSMShader>();
#else
		auto shader_data = make_shared<DirectionalLightShadowMapShader>();
#endif
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else
	{
		assert(0 && "shader type not supported");
	}
	return nullptr;
}
