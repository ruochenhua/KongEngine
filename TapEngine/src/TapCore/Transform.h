#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Tap
{
    class CTransform
    {
    public:
        CTransform();
        glm::vec3 GetPosition() const;
        glm::quat GetRotation() const;
        glm::vec3 GetScale() const;

		glm::mat4 GetMatrix() const;

        void SetPosition(const glm::vec3& pos);
        void SetRotation(const glm::quat& rot);
		void SetRotation(const glm::vec3& rot_eular);
        void SetScale(const glm::vec3& scale);

		glm::vec3 Transform(const glm::vec3& vec) const;
    private:
        glm::vec3 m_Pos;    //position
        glm::quat m_Rot;    //rotation
        glm::vec3 m_Scale;  //scale, 也许不应该有scale？
    };
};