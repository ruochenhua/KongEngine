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
		: m_center(center), m_eye(eye), m_up(up), m_screenInfo(screen_info)
	{}

	glm::mat4 GetProjectionMatrix() const;
	glm::mat4 GetViewMatrix() const;

private:
	glm::vec3 m_center;
	glm::vec3 m_eye;
	glm::vec3 m_up;

	SScreenInfo m_screenInfo;
};


class CRender
{
public:
	CRender() : m_pWindow(nullptr), m_pCamera(nullptr) { }
	int Init();
	int Update();

	SRenderInfo AddModel(CModel* model, const std::string shader_paths[2]);

	GLuint LoadShaders(const std::string& vs, const std::string& fs);

private:
	void RenderModel(const SRenderInfo& render_info) const;

private:
	GLFWwindow* m_pWindow;
	CCamera* m_pCamera;
	std::vector<SRenderInfo> m_vRenderInfo;
};