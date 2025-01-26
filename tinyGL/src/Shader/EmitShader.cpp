#include "EmitShader.h"

#include "Render/RenderModule.hpp"
#include "Scene.hpp"
using namespace Kong;
using namespace glm;
using namespace std;

void EmitShader::UpdateRenderData(const SMaterial& render_material,
			const SSceneLightInfo& scene_render_info)
{
	SetVec4("albedo", render_material.albedo);
	// GLuint null_tex_id = CRender::GetNullTexId();
	// glActiveTexture(GL_TEXTURE0);
	// GLuint diffuse_tex_id = render_material.diffuse_tex_id != 0 ? render_material.diffuse_tex_id : null_tex_id;
	// glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);
}

void EmitShader::InitDefaultShader()
{
	shader_path_map = {
		{vs, CSceneLoader::ToResourcePath("shader/emit.vert")},
		{fs, CSceneLoader::ToResourcePath("shader/emit.frag")},
	};
	shader_id = Shader::LoadShaders(shader_path_map);
    
	assert(shader_id, "Shader load failed!");
}
