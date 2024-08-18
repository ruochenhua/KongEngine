#pragma once
#include "Component/Mesh/BoxShape.h"
#include "Common.h"
#include "RenderCommon.h"


namespace tinyGL
{
	class CSkyBox
	{
	public:
		void Init();

		void Render(const glm::mat4& mvp);
		GLuint GetTextureId() const {return cube_map_id;}
	private:
		GLuint shader_id = GL_NONE;
		GLuint sphere_to_cube_shader_id = GL_NONE;
		GLuint cube_map_id = GL_NONE;

		shared_ptr<CBoxShape> box_mesh;

		// 球形环境贴图转换为立方体贴图的帧缓冲
		GLuint sphere_to_cube_fbo = GL_NONE;
		GLuint sphere_to_cube_rbo = GL_NONE;
		GLuint sphere_map_texture = GL_NONE;
	};
}
