#include "Camera.h"

#include <GLFW/glfw3.h>
// #include "message.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

GLFWwindow* CRender::s_pWindow = nullptr;

mat4 CCamera::GetProjectionMatrix() const
{
	return perspective(m_screenInfo._fov, m_screenInfo._aspect_ratio, m_screenInfo._near, m_screenInfo._far);
}

mat4 CCamera::GetViewMatrix() const
{
	return lookAt(m_center, m_eye, m_up);
}

mat4 CCamera::GetViewMatrixNoTranslate() const
{
	return lookAt(vec3(0, 0, 0), GetDirection(), m_up);
}

vec3 CCamera::GetDirection() const
{
	return normalize(m_eye - m_center);	
}

vec3 CCamera::GetPosition() const
{
	return m_center;
}

glm::mat4 CCamera::UpdateRotation()
{
	glm::mat4 rot_mat = glm::identity<mat4>();

	double x_pos, y_pos;
	glfwGetCursorPos(CRender::s_pWindow, &x_pos, &y_pos);
	
	double delta_x = x_pos - m_cursorX, delta_y = y_pos - m_cursorY;
	m_cursorX = x_pos; m_cursorY = y_pos;

	rot_mat = glm::rotate(rot_mat, (float)-delta_x * 0.02f, vec3(0.0, 1.0, 0.0));

	vec3 dir = m_eye - m_center;
	vec3 right = cross(dir, m_up);
	rot_mat = glm::rotate(rot_mat, (float)-delta_y * 0.02f, normalize(right));

	return rot_mat;
}

void CCamera::Update()
{
	glm::mat4 transform_mat = glm::identity<mat4>();

	if (m_updateRotation)
	{
		transform_mat = UpdateRotation();
	}

	//update translate
	transform_mat[3][0] = m_moveVec.x;
	transform_mat[3][1] = m_moveVec.y;
	transform_mat[3][2] = m_moveVec.z;

	m_eye = vec3(transform_mat * vec4(m_eye, 1));
	m_center = vec3(transform_mat * vec4(m_center, 1));

	m_moveVec = vec3(0, 0, 0);
}

void CCamera::InitControl()
{
	// CMessage::BindKeyToFunction(GLFW_KEY_W, GLFW_PRESS, MoveForward);
	// CMessage::BindKeyToFunction(GLFW_KEY_S, GLFW_PRESS, MoveBackward);
	// CMessage::BindKeyToFunction(GLFW_KEY_A, GLFW_PRESS, MoveLeft);
	// CMessage::BindKeyToFunction(GLFW_KEY_D, GLFW_PRESS, MoveRight);
	// CMessage::BindKeyToFunction(GLFW_KEY_W, GLFW_REPEAT, MoveForward);
	// CMessage::BindKeyToFunction(GLFW_KEY_S, GLFW_REPEAT, MoveBackward);
	// CMessage::BindKeyToFunction(GLFW_KEY_A, GLFW_REPEAT, MoveLeft);
	// CMessage::BindKeyToFunction(GLFW_KEY_D, GLFW_REPEAT, MoveRight);
	//
	// CMessage::BindMouseToFunction(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, RotateStart);
	// CMessage::BindMouseToFunction(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, RotateEnd);
}

void CCamera::MoveForward()
{
	vec3 dir = m_eye - m_center;
	m_moveVec += dir * 0.1f;
}

void CCamera::MoveBackward()
{
	vec3 dir = m_eye - m_center;
	m_moveVec -= dir * 0.1f;
}

void CCamera::MoveLeft()
{
	vec3 dir = m_eye - m_center;
	vec3 right = cross(dir, m_up);

	m_moveVec -= right * 0.1f;
}

void CCamera::MoveRight()
{
	vec3 dir = m_eye - m_center;
	vec3 right = cross(dir, m_up);

	m_moveVec += right * 0.1f;
}

void CCamera::RotateStart()
{
	glfwGetCursorPos(CRender::s_pWindow, &m_cursorX, &m_cursorX);
	m_updateRotation = true;
}

void CCamera::RotateEnd()
{
	m_updateRotation = false;
}