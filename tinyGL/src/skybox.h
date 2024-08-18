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

	private:
		GLuint shader_id = GL_NONE;
		GLuint cube_map_id = GL_NONE;

		shared_ptr<CBoxShape> box_mesh;
	};
}
