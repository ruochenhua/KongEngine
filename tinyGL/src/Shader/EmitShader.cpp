#include "EmitShader.h"

#include "render.h"
#include "Scene.h"
using namespace Kong;
using namespace glm;
using namespace std;

void EmitShader::UpdateRenderData(const SRenderInfo& render_info,
			const SSceneRenderInfo& scene_render_info)
{
	glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO

	// 材质属性
	SetVec4("albedo", render_info.material.albedo);
	
	GLuint null_tex_id = CRender::GetNullTexId();
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	glActiveTexture(GL_TEXTURE1);
	GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, specular_map_id);
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
