#include "Camera.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>

#include "Engine.h"
#include "message.h"
// #include "message.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

mat4 CCamera::GetProjectionMatrix() const
{
	return perspective(m_screenInfo._fov, m_screenInfo._aspect_ratio, m_screenInfo._near, m_screenInfo._far);
}

mat4 CCamera::GetViewMatrix() const
{
	return lookAt(m_center, m_center+m_front, m_up);
}

mat4 CCamera::GetViewMatrixNoTranslate() const
{
	return lookAt(vec3(0, 0, 0), m_front, m_up);
}

vec3 CCamera::GetDirection() const
{
	return m_front;	
}

vec3 CCamera::GetPosition() const
{
	return m_center;
}

void CCamera::UpdateRotation(double delta)
{
	auto window = Engine::GetRenderWindow();
	double x_pos, y_pos;
	glfwGetCursorPos(window, &x_pos, &y_pos);	
	double delta_x = x_pos - m_cursorX;
	float delta_y = y_pos - m_cursorY;
	
	m_cursorX = x_pos;
	m_cursorY = y_pos;

	if (!m_updateRotation)
	{
		return;
	}
	
	m_yaw -= delta_x*delta*rotate_speed;
	m_pitch += delta_y*delta*rotate_speed;

	m_pitch = clamp(m_pitch, -89.f, 89.f);
	// printf("=====\n yaw value %f, xpos %f, delta %f\n", m_yaw, x_pos, delta_x);
	// printf("pitch value %f, ypos %f, delta %f\n", m_pitch, y_pos, delta_y);
	
	vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = normalize(front);
	
	// glm::mat4 rot_mat = glm::identity<mat4>();
	// rot_mat = glm::rotate(rot_mat, (float)(-delta_x * delta), vec3(0.0, 1.0, 0.0));
	//
	// vec3 dir = m_front;
	// vec3 right = cross(dir, m_up);
	// rot_mat = glm::rotate(rot_mat, (float)(-delta_y * delta), normalize(right));
}

void CCamera::Update(double delta)
{
	m_screenInfo._aspect_ratio = Engine::GetEngine().GetAspectRatio();
	UpdateRotation(delta);
	auto render_window = Engine::GetRenderWindow();
	// double xpos, ypos;
	// glfwGetCursorPos(render_window, &xpos, &ypos);
	// printf("pitch xpos %f, ypos %f, delta %f\n", (float)xpos, (float)ypos, (float)delta);
	if(glfwGetKey(render_window, GLFW_KEY_W) == GLFW_PRESS
		|| glfwGetKey(render_window, GLFW_KEY_W) == GLFW_REPEAT)
	{
		MoveForward();
	}

	if(glfwGetKey(render_window, GLFW_KEY_A) == GLFW_PRESS
		|| glfwGetKey(render_window, GLFW_KEY_A) == GLFW_REPEAT)
	{
		MoveLeft();
	}

	if(glfwGetKey(render_window, GLFW_KEY_S) == GLFW_PRESS
		|| glfwGetKey(render_window, GLFW_KEY_S) == GLFW_REPEAT)
	{
		MoveBackward();
	}

	if(glfwGetKey(render_window, GLFW_KEY_D) == GLFW_PRESS
		|| glfwGetKey(render_window, GLFW_KEY_D) == GLFW_REPEAT)
	{
		MoveRight();
	}

	if(glfwGetMouseButton(render_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		RotateStart();
	}

	if(glfwGetMouseButton(render_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		RotateEnd();
	}
	
	//update translate
	m_center += m_moveVec * float(delta*move_speed);
	// transform_mat[3][0] = m_moveVec.x;
	// transform_mat[3][1] = m_moveVec.y;
	// transform_mat[3][2] = m_moveVec.z;
	//
	// m_eye = vec3(transform_mat * vec4(m_eye, 1));
	// m_center = vec3(transform_mat * vec4(m_center, 1));

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
	m_moveVec += m_front * 0.1f;
}

void CCamera::MoveBackward()
{
	m_moveVec -= m_front * 0.1f;
}

void CCamera::MoveLeft()
{
	vec3 right = cross(m_front, m_up);

	m_moveVec -= right * 0.1f;
}

void CCamera::MoveRight()
{
	vec3 right = cross(m_front, m_up);

	m_moveVec += right * 0.1f;
}

void CCamera::RotateStart()
{
	// if(m_updateRotation)
	// 	return;
	//
	// auto window = Engine::GetRenderWindow();
	// glfwGetCursorPos(window, &m_cursorX, &m_cursorY);
	// glfwGetCursorPos(window, &m_cursorX, &m_cursorY);
	m_updateRotation = true;
}

void CCamera::RotateEnd()
{
	m_updateRotation = false;
}
