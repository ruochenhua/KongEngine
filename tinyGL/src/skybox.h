#pragma once
#include "Common.h"
#include "RenderCommon.h"

namespace tinyGL
{
	class TGAImage;

	struct SCubeMapTexture {
	void Bind(GLenum texture_unit);
	void Load(const std::vector<std::string>& tex_path_vec,
		const std::vector<unsigned int>& tex_type_vec);

	GLuint cube_map_id;
	TGAImage* texture_img[6];
};

struct SSkyBoxMesh {
	void Init(const std::vector<std::string>& tex_path_vec,
		const std::vector<unsigned int>& tex_type_vec);
	
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> texcoords;

	std::vector<GLfloat> v_geo_bb;

	SRenderInfo render_info;
	GLuint shader_id = 0;
	SCubeMapTexture cube_map_tex;
};

class CSkyBox
{
public:
	void Init();

	void Render(const glm::mat4& mvp);

private:
	//SSkyBoxTech m_Tech;
	SSkyBoxMesh m_BoxMesh;
	GLuint m_programID;
};
}
