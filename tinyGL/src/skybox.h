#pragma once
#include "common.h"
namespace tinyGL
{
struct SCubeMapTexture {
	void Bind(GLenum texture_unit);
	void Load(const std::vector<std::string>& tex_path_vec,
		const std::vector<unsigned int>& tex_type_vec);
	//天空盒的贴图
	//GLuint _aTextureID[6];
	GLuint _CubeMapID;
	TGAImage* _aTextureImg[6];
};

struct SSkyBoxMesh {
	void Init(const std::vector<std::string>& tex_path_vec,
		const std::vector<unsigned int>& tex_type_vec);

	//盒子的mesh
	std::vector<GLfloat> _vertices;
	std::vector<GLfloat> _texcoords;

	std::vector<GLfloat> _vGeoBB;

	SRenderInfo _render_info;

	SCubeMapTexture _cube_map_tex;
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