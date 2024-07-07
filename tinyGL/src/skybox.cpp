#include "skybox.h"
#include "tgaimage.h"
#include "render.h"

namespace tinyGL
{
void SCubeMapTexture::Bind(GLenum texture_unit)
{
	glActiveTexture(texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _CubeMapID);
}

void SCubeMapTexture::Load(const std::vector<std::string>& tex_path_vec, const std::vector<unsigned int>& tex_type_vec)
{
	assert(tex_path_vec.size() == 6);
	assert(tex_type_vec.size() == 6);

	//create texture
	glGenTextures(1, &_CubeMapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _CubeMapID);


	for (int i = 0; i < 6; ++i)
	{
		auto tex_img = new TGAImage;
		assert(tex_img->read_tga_file(tex_path_vec[i].c_str()));

		_aTextureImg[i] = tex_img;

		//filter
		int tex_width = tex_img->get_width();
		int tex_height = tex_img->get_height();
		unsigned char* tex_data = tex_img->buffer();

		glTexImage2D(tex_type_vec[i], 0, GL_RGB, tex_width, tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, (void*)tex_data);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_2D);


		// 		//create buffer
		// 		glGenBuffers(1, &m_aTextureBuffer[i]);
		// 		glBindBuffer(GL_ARRAY_BUFFER, m_aTextureBuffer[i]);
		// 		glBufferData(GL_ARRAY_BUFFER, sizeof(SKY_BOX_TEXCOORD), &SKY_BOX_TEXCOORD[0], GL_STATIC_DRAW);
	}
}

void SSkyBoxMesh::Init(const std::vector<std::string>& tex_path_vec,
	const std::vector<unsigned int>& tex_type_vec)
{
	//create box mesh
	_vertices.clear();
	_vertices = {
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,

		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,

		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,

		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
	};

	// 	_texcoords.clear();
	// 	_texcoords = {
	// 		0.0f, 0.0f,	// A
	// 		1.0f, 0.0f,	// B
	// 		1.0f, 1.0f,	// C
	// 		1.0f, 1.0f,	// C
	// 		0.0f, 1.0f,	// D
	// 		0.0f, 0.0f,	// A
	// 
	// 		0.0f, 0.0f,	// E
	// 		0.0, 1.0f,   // H
	// 		1.0f, 1.0f,	// G
	// 		1.0f, 1.0f,	// G
	// 		1.0f, 0.0f,	// F
	// 		0.0f, 0.0f,	// E
	// 
	// 		0.0f, 1.0f,	// D
	// 		1.0, 1.0f,   // H
	// 		1.0f, 0.0f,	// E
	// 		1.0f, 0.0f,	// E
	// 		0.0f, 0.0f,	// A
	// 		0.0f, 1.0f,	// D
	// 
	// 		1.0f, 0.0f,	// F
	// 		1.0f, 1.0f,	// G
	// 		0.0f, 1.0f,	// C
	// 		0.0f, 1.0f,	// C
	// 		0.0f, 0.0f,	// B
	// 		1.0f, 0.0f,	// F
	// 
	// 		1.0f, 1.0f,	// G
	// 		0.0, 1.0f,   // H
	// 		0.0f, 0.0f,	// D
	// 		0.0f, 0.0f,	// D
	// 		1.0f, 0.0f,	// C
	// 		1.0f, 1.0f,	// G
	// 
	// 		0.0f, 0.0f,	// A
	// 		0.0f, 1.0f,// E
	// 		1.0f, 1.0f,	// F
	// 		1.0f, 1.0f,	// F
	// 		1.0f, 0.0f,	// B
	// 		0.0f, 0.0f,	// A
	// 	};

		// 指定包围盒的顶点属性 位置
	_vGeoBB.clear();
	_vGeoBB = {
		// 背面
		-1.0f, 1.0f, -1.0f,		// A
		-1.0f, -1.0f, -1.0f,	// B
		1.0f, -1.0f, -1.0f,		// C
		1.0f, -1.0f, -1.0f,		// C
		1.0f, 1.0f, -1.0f,		// D
		-1.0f, 1.0f, -1.0f,		// A

		// 左侧面
		-1.0f, -1.0f, 1.0f,		// E
		-1.0f, -1.0f, -1.0f,	// B
		-1.0f, 1.0f, -1.0f,		// A
		-1.0f, 1.0f, -1.0f,		// A
		-1.0f, 1.0f, 1.0f,		// F
		-1.0f, -1.0f, 1.0f,		// E

		// 右侧面
		1.0f, -1.0f, -1.0f,		// C
		1.0f, -1.0f, 1.0f,		// G
		1.0f, 1.0f, 1.0f,		// H
		1.0f, 1.0f, 1.0f,		// H
		1.0f, 1.0f, -1.0f,		// D
		1.0f, -1.0f, -1.0f,		// C

		// 正面
		-1.0f, -1.0f, 1.0f,  // E
		-1.0f, 1.0f, 1.0f,  // F
		1.0f, 1.0f, 1.0f,  // H
		1.0f, 1.0f, 1.0f,  // H
		1.0f, -1.0f, 1.0f,  // G
		-1.0f, -1.0f, 1.0f,  // E

		// 顶面
		-1.0f, 1.0f, -1.0f,  // A
		1.0f, 1.0f, -1.0f,  // D
		1.0f, 1.0f, 1.0f,  // H
		1.0f, 1.0f, 1.0f,  // H
		-1.0f, 1.0f, 1.0f,  // F
		-1.0f, 1.0f, -1.0f,  // A

		// 底面
		-1.0f, -1.0f, -1.0f,  // B
		-1.0f, -1.0f, 1.0f,   // E
		1.0f, -1.0f, 1.0f,    // G
		1.0f, -1.0f, 1.0f,    // G
		1.0f, -1.0f, -1.0f,   // C
		-1.0f, -1.0f, -1.0f,  // B
	};

	glGenVertexArrays(1, &_render_info.vertexArrayId);
	glBindVertexArray(_render_info.vertexArrayId);

	glGenBuffers(1, &_render_info.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _render_info.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*_vertices.size(), &_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _render_info.vertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	// 	glGenBuffers(1, &_render_info._texture_buffer);
	// 	glBindBuffer(GL_ARRAY_BUFFER, _render_info._texture_buffer);
	// 	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*_vertices.size(), &_vertices[0], GL_STATIC_DRAW);

	_render_info._program_id = CRender::LoadShaders("../../../../resource/shader/skybox_vert.shader", "../../../../resource/shader/skybox_frag.shader");
	_render_info._vertex_size = _vertices.size();

	_cube_map_tex.Load(tex_path_vec, tex_type_vec);
}

void CSkyBox::Init()
{
	std::vector<std::string> tex_path_vec = {
		"../../../../resource/sky_box/dark_sky/darkskies_bk.tga",
		"../../../../resource/sky_box/dark_sky/darkskies_dn.tga",
		"../../../../resource/sky_box/dark_sky/darkskies_ft.tga",
		"../../../../resource/sky_box/dark_sky/darkskies_lf.tga",
		"../../../../resource/sky_box/dark_sky/darkskies_rt.tga",
		"../../../../resource/sky_box/dark_sky/darkskies_up.tga",
	};
	std::vector<unsigned int> tex_type_vec = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	};
	
	m_BoxMesh.Init(tex_path_vec, tex_type_vec);
}

void CSkyBox::Render(const glm::mat4& mvp)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	auto& mesh_info = m_BoxMesh._render_info;
	glUseProgram(mesh_info._program_id);
	glBindVertexArray(mesh_info.vertexArrayId);
	GLuint matrix_id = glGetUniformLocation(mesh_info._program_id, "MVP");
	glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_BoxMesh._cube_map_tex._CubeMapID);

	glUniform1i(glGetUniformLocation(mesh_info._program_id, "skybox"), 0);
	// 	glEnableVertexAttribArray(1);
	// 	glBindBuffer(GL_ARRAY_BUFFER, mesh_info._texture_buffer);
	// 	glVertexAttribPointer(
	// 		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	// 		2,                  // size
	// 		GL_FLOAT,           // type
	// 		GL_FALSE,           // normalized?
	// 		0,                  // stride
	// 		(void*)0            // array buffer offset
	// 	);

	glDrawArrays(GL_TRIANGLES, 0, mesh_info._vertex_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glBindVertexArray(GL_NONE);
}
}