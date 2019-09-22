#include "render.h"
#include "model.h"
#include "tgaimage.h"

using namespace glm;
using namespace std;

mat4 CCamera::GetProjectionMatrix() const
{
	return perspective(m_screenInfo._fov, m_screenInfo._aspect_ratio, m_screenInfo._near, m_screenInfo._far);
}

mat4 CCamera::GetViewMatrix() const
{
	//look at will not calculate translation, need to do it ourselves
	return translate(lookAt(m_eye, m_center, m_up), m_center);	
}

int CRender::Init()
{
	
	glewExperimental = true;
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	m_pWindow; // (In the accompanying source code, this variable is global for simplicity)
	m_pWindow = glfwCreateWindow(1024, 768, "tinyGL", NULL, NULL);
	if (m_pWindow == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(m_pWindow); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(m_pWindow, GLFW_STICKY_KEYS, GL_TRUE);

	SScreenInfo screen_info(glm::radians(45.0f), 1024.0f / 768.0f, 0.5f, 300.0f);
	m_pCamera = new CCamera(vec3(0.0f, 0.0f, -4.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), screen_info);


	return 0;
}

int CRender::Update()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//opengl32.dll

	for (auto& render_info : m_vRenderInfo)
	{
		RenderModel(render_info);
	}

	// Swap buffers
	glfwSwapBuffers(m_pWindow);
	glfwPollEvents();
	return 0;
}

SRenderInfo CRender::AddModel(CModel* model, const std::string shader_paths[2])
{
	std::vector<float> vertices = model->GetVertices();
	SRenderInfo info;

	glGenVertexArrays(1, &info._vertex_array_id);
	glBindVertexArray(info._vertex_array_id);

	//init vertex buffer
	glGenBuffers(1, &info._vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, info._vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
	
	TGAImage* tex_img = model->GetTextureImage();
	if (tex_img)
	{
		std::vector<float> tex_coords = model->GetTextureCoords();
		glGenBuffers(1, &info._texture_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, info._texture_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
	}

	info._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
	info._vertex_size = vertices.size();
	info._texture_img = model->GetTextureImage();

	m_vRenderInfo.push_back(info);
	return info;
}

void CRender::RenderModel(const SRenderInfo& render_info) const
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(render_info._program_id);	

	mat4 Model = mat4(1.0f);
	mat4 projection = m_pCamera->GetProjectionMatrix();
	mat4 mvp = projection * m_pCamera->GetViewMatrix() * Model; // 
	GLuint matrix_id = glGetUniformLocation(render_info._program_id, "MVP");

	TGAImage* texture_img = render_info._texture_img;
	assert(texture_img);

	glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

	//vertex buffer
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, render_info._vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	int tex_width = texture_img->get_width();
	int tex_height = texture_img->get_height();
	unsigned char* tex_data = texture_img->buffer();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, (void*)tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//texture buffer
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, render_info._texture_buffer);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);


	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, render_info._vertex_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

GLuint CRender::LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path) 
{
	// Create the shaders
	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string vs_code;
	std::ifstream vs_stream(vertex_file_path, std::ios::in);
	if (vs_stream.is_open()) {
		std::stringstream sstr;
		sstr << vs_stream.rdbuf();
		vs_code = sstr.str();
		vs_stream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path.c_str());
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string fs_code;
	std::ifstream fs_stream(fragment_file_path, std::ios::in);
	if (fs_stream.is_open()) {
		std::stringstream sstr;
		sstr << fs_stream.rdbuf();
		fs_code = sstr.str();
		fs_stream.close();
	}

	GLint result = GL_FALSE;
	int info_log_length;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path.c_str());
	char const * vs_ptr = vs_code.c_str();
	glShaderSource(vs_id, 1, &vs_ptr, NULL);
	glCompileShader(vs_id);

	// Check Vertex Shader
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vs_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> vs_error_msg(info_log_length + 1);
		glGetShaderInfoLog(vs_id, info_log_length, NULL, &vs_error_msg[0]);
		printf("%s\n", &vs_error_msg[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path.c_str());
	char const * fs_ptr = fs_code.c_str();
	glShaderSource(fs_id, 1, &fs_ptr, NULL);
	glCompileShader(fs_id);

	// Check Fragment Shader
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fs_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> fs_error_msg(info_log_length + 1);
		glGetShaderInfoLog(fs_id, info_log_length, NULL, &fs_error_msg[0]);
		printf("%s\n", &fs_error_msg[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint prog_id = glCreateProgram();
	glAttachShader(prog_id, vs_id);
	glAttachShader(prog_id, fs_id);
	glLinkProgram(prog_id);

	// Check the program
	glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
	glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> prog_error_msg(info_log_length + 1);
		glGetProgramInfoLog(prog_id, info_log_length, NULL, &prog_error_msg[0]);
		printf("%s\n", &prog_error_msg[0]);
	}

	glDetachShader(prog_id, vs_id);
	glDetachShader(prog_id, fs_id);

	glDeleteShader(vs_id);
	glDeleteShader(fs_id);

	return prog_id;
}