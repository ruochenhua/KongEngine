#include "Shader.h"

using namespace tinyGL;

Shader::Shader(const SRenderResourceDesc& render_resource_desc)
{
    Init(render_resource_desc.shader_paths);
}

void Shader::Init(const map<EShaderType, string>& shader_path_cache)
{
    shader_id = Shader::LoadShaders(shader_path_cache);
}

void Shader::Use() const
{
    assert(shader_id, "Shader not loaded yet!");
    glUseProgram(shader_id);
}
