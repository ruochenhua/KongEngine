#include "render.h"

#include "Camera.h"
#include "model.h"
#include "tgaimage.h"
#include "message.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

int CRender::Init()
{
	InitRender();
	InitCameraControl();

	// 初始化天空盒
	m_SkyBox.Init();
	//init show map
	glGenFramebuffers(1, &m_FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);

	// ? 深度贴图相关
	glGenTextures(1, &m_DepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture, 0);
	// 清空buffer
	glDrawBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return -1;

	// 加载shader，编译并构建对应的程序
	m_ShadowMapProgramID = LoadShaders("../../../../resource/shader/shadowmap_vert.shader",
		"../../../../resource/shader/shadowmap_frag.shader");

	m_DepthMatrixID = glGetUniformLocation(m_ShadowMapProgramID, "depth_mvp");
	return 0;
}

int CRender::InitRender()
{
	glewExperimental = true;
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}
	// 初始化opengl
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

	// Open a window and create its OpenGL context
	// (In the accompanying source code, this variable is global for simplicity)
	s_pWindow = glfwCreateWindow(1024, 768, "tinyGL", NULL, NULL);
	if (s_pWindow == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
		glfwTerminate();
		return -1;
	}
	//set keyboard and mouse call back
	glfwSetKeyCallback(s_pWindow, CMessage::KeyCallback);
	glfwSetMouseButtonCallback(s_pWindow, CMessage::MouseButtonCallback);

	glfwMakeContextCurrent(s_pWindow); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(s_pWindow, GLFW_STICKY_KEYS, GL_TRUE);

	return 0;
}

int CRender::InitCameraControl()
{
	mainCamera = new CCamera(vec3(0.0f, 0.0f, -4.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	mainCamera->InitControl();
	return 0;
}

int CRender::Update()
{
	//update camera
	mainCamera->Update();		

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	glViewport(0, 0, 1024, 1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	for (auto& render_info : m_vRenderInfo)
	{
		RenderShadowMap(render_info);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//opengl32.dll	
	glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	RenderSkyBox();
	for (auto& render_info : m_vRenderInfo)
	{
		RenderModel(render_info);
	}
	

	// Swap buffers
	glfwSwapBuffers(s_pWindow);
	glfwPollEvents();
	return 1;
}

SRenderInfo CRender::AddModel(CModel* model, const std::string shader_paths[2])
{
	std::vector<float> vertices = model->GetVertices();
	SRenderInfo info;

	glGenVertexArrays(1, &info.vertexArrayId);
	glBindVertexArray(info.vertexArrayId);

	//init vertex buffer
	glGenBuffers(1, &info.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, info.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	//vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,  info.vertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);
	
	//normal buffer
	std::vector<float> normals = model->GetNormals();
	glGenBuffers(1, &info._normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, info._normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), &normals[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(
		2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(1);
	
	TGAImage* tex_img = model->GetTextureImage();
	if (tex_img)
	{
		std::vector<float> tex_coords = model->GetTextureCoords();
		glGenBuffers(1, &info._texture_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, info._texture_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
	
		glBindBuffer(GL_ARRAY_BUFFER, info._texture_buffer);
		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		glEnableVertexAttribArray(2);
	}
	// index buffer
	std::vector<unsigned int> indices = model->GetIndices();
	glGenBuffers(1, &info.indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
	glBindVertexArray(GL_NONE);
	
	info._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
	info._vertex_size = vertices.size();
	info._indices_count = indices.size();
	info._texture_img = model->GetTextureImage();

	m_vRenderInfo.push_back(info);
	return info;
}

void CRender::RenderSkyBox()
{
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrixNoTranslate(); //
	m_SkyBox.Render(mvp);
}

void CRender::RenderShadowMap(const SRenderInfo& render_info)
{		
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

	// 使用阴影贴图的shader
	glUseProgram(m_ShadowMapProgramID);	

	// Compute the MVP matrix from the light's point of view
	//1024.0f / 768.0f, 0.1f, 500.0f
	mat4 depth_proj_mat = ortho<float>(-10, 10, -10, 10, -10, 20);
	mat4 depth_view_mat = lookAt(m_LightDir+m_LightPos, m_LightPos, glm::vec3(0, 1, 0));

	mat4 depth_model_mat = mat4(1.0);
	m_DepthMVP = depth_proj_mat * depth_view_mat * depth_model_mat;

	// in the "MVP" uniform
	glUniformMatrix4fv(m_DepthMatrixID, 1, GL_FALSE, &m_DepthMVP[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, render_info.vertexBuffer);
	glVertexAttribPointer(
		0,  // The attribute we want to configure
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);	

	glDrawArrays(GL_TRIANGLES, 0, render_info._vertex_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle	

	glDisableVertexAttribArray(0);
}

void CRender::RenderModel(const SRenderInfo& render_info) const
{	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(render_info._program_id);
	glBindVertexArray(render_info.vertexArrayId);	// 绑定VAO
	
	mat4 Model = mat4(1.0f);
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrix() * Model; //
	GLuint matrix_id = glGetUniformLocation(render_info._program_id, "MVP");
	glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

	GLuint light_shininess_id = glGetUniformLocation(render_info._program_id, "shininess");
	glUniform1f(light_shininess_id, render_info._material._shininess);

	GLuint cam_pos_id = glGetUniformLocation(render_info._program_id, "cam_pos");
	glUniform3fv(cam_pos_id, 1, &mainCamera->GetPosition()[0]);

	GLuint light_dir_id = glGetUniformLocation(render_info._program_id, "light_dir");
	glUniform3fv(light_dir_id, 1, &m_LightDir[0]);
	
	GLuint light_color_id = glGetUniformLocation(render_info._program_id, "light_color");
	glUniform3fv(light_color_id, 1, &m_LightColor[0]);

	//use shadow map
	mat4 bias_mat(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	mat4 depth_bias_mvp = bias_mat * m_DepthMVP;

	GLuint depth_bias_id = glGetUniformLocation(render_info._program_id, "depth_bias_mvp");
	glUniformMatrix4fv(depth_bias_id, 1, GL_FALSE, &depth_bias_mvp[0][0]);
		
	TGAImage* texture_img = render_info._texture_img;
	assert(texture_img);

	// FIXME: 不用每次都生成吧
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	int tex_width = texture_img->get_width();
	int tex_height = texture_img->get_height();
	unsigned char* tex_data = texture_img->buffer();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_BGR, GL_UNSIGNED_BYTE, (void*)tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);	
	GLuint sm_id = glGetUniformLocation(render_info._program_id, "shadowmap_id");
	glUniform1i(sm_id, m_DepthTexture);
	
	// Draw the triangle !
	// glDrawArrays(GL_TRIANGLES, 0, render_info._vertex_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_info.indexBuffer);
	glDrawElements(GL_TRIANGLES, render_info._indices_count, GL_UNSIGNED_INT, 0);

	glBindVertexArray(GL_NONE);	// 解绑VAO
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
