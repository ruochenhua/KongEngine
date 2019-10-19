#pragma once
#include "common.h"

class CModel;
struct SScreenInfo
{
	SScreenInfo(float fov, float aspect_ratio, float near, float far) :
		_fov(fov), _aspect_ratio(aspect_ratio), _near(near), _far(far)
	{}

	float _fov;
	float _aspect_ratio;
	float _near, _far;
};


class CCamera
{
public:
	CCamera(const glm::vec3& center, const glm::vec3& eye, const glm::vec3& up, const SScreenInfo& screen_info)
		: m_center(center), m_eye(eye), m_up(up), m_moveVec(glm::vec3(0,0,0)), m_screenInfo(screen_info)
		, m_updateRotation(false), m_cursorX(0.0), m_cursorY(0.0)
	{}

	void Update();

	glm::mat4 GetProjectionMatrix() const;
	glm::mat4 GetViewMatrix() const;

public:
	//camera control
	void InitControl();
	static void MoveForward();
	static void MoveBackward();
	static void MoveLeft();
	static void MoveRight();
	static void RotateStart();
	static void RotateEnd();
	
private:
	glm::mat4 UpdateRotation();
	bool m_updateRotation;
	double m_cursorX, m_cursorY;

	glm::vec3 m_center;
	glm::vec3 m_eye;
	glm::vec3 m_up;

	glm::vec3 m_moveVec;
	glm::vec3 m_rotateVec;

	SScreenInfo m_screenInfo;
};

class CRender
{
public:
	static GLFWwindow* s_pWindow;

public:
	CRender() { }
	int Init();

	int Update();

	SRenderInfo AddModel(CModel* model, const std::string shader_paths[2]);

	GLuint LoadShaders(const std::string& vs, const std::string& fs);

private:
	int InitRender();
	int InitCameraControl();

	void RenderModel(const SRenderInfo& render_info) const;

private:
	std::vector<SRenderInfo> m_vRenderInfo;

};