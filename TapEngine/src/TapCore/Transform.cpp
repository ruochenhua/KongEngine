#include "stdafx.h"
#include "Transform.h"
#include <glm/gtx/quaternion.hpp>

namespace Tap
{
    CTransform::CTransform()
    : m_Pos(glm::vec3(0.0,0.0,0.0))
    , m_Rot(glm::quat(1.0,0.0,0.0,0.0))
    , m_Scale(glm::vec3(1.0,1.0,1.0))
    {

    }

    glm::vec3 CTransform::GetPosition() const
    {
        return m_Pos;
    }

    glm::quat CTransform::GetRotation() const
    {
        return m_Rot;
    }

    glm::vec3 CTransform::GetScale() const
    {
        return m_Scale;
    }

	glm::mat4 CTransform::GetMatrix() const
	{		
		//将srt结合成一个转置矩阵
		glm::mat4 transformMatrix = glm::identity<glm::mat4>();
		glm::mat4 rot_mat = glm::toMat4(m_Rot);										

		transformMatrix *= rot_mat;

		transformMatrix[3] = glm::vec4(m_Pos.x, m_Pos.y, m_Pos.z, 1);		
		transformMatrix[0][0] *= m_Scale[0];
		transformMatrix[1][1] *= m_Scale[1];
		transformMatrix[2][2] *= m_Scale[2];

		return transformMatrix;
	}

    void CTransform::SetPosition(const glm::vec3& pos)
    {
        m_Pos = pos;
    }

    void CTransform::SetRotation(const glm::quat& rot)
    {
        m_Rot = glm::normalize(rot);
    }

	void CTransform::SetRotation(const glm::vec3& rot_eular)
	{
		m_Rot = glm::normalize(glm::quat(rot_eular));
		
	}

    void CTransform::SetScale(const glm::vec3& scale)
    {
        m_Scale = scale;
    }

	glm::vec3 CTransform::Transform(const glm::vec3& vec) const
	{
		glm::vec4 value = GetMatrix() * glm::vec4(vec, 1.0f);
		return glm::vec3(value.x, value.y, value.z);
	}
};