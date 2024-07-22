#pragma once
#include "common.h"
namespace tinyGL
{
	class TGAImage;

	struct SCubeMapTexture {
	void Bind(GLenum texture_unit);
	void Load(const std::vector<std::string>& tex_path_vec,
		const std::vector<unsigned int>& tex_type_vec);
	//天空盒的贴图
	//GLuint _aTextureID[6];
	GLuint cube_map_id;
	TGAImage* texture_img[6];
};

struct SSkyBoxMesh {
	void Init(const std::vector<std::string>& tex_path_vec,
		const std::vector<unsigned int>& tex_type_vec);

	//盒子的mesh
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> texcoords;

	std::vector<GLfloat> v_geo_bb;

	SRenderInfo render_info;

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
	//天空盒的shader
	GLuint m_programID;
};
}