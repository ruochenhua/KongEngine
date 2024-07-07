#pragma once
#include <GLM/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

#include "render.h"

namespace tinyGL
{
    using namespace glm;
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
        CCamera(const vec3& center, const vec3& eye, const vec3& up)
            : m_center(center), m_eye(eye), m_up(up), m_moveVec(vec3(0, 0, 0))
            , m_updateRotation(false), m_cursorX(0.0), m_cursorY(0.0)
        {
            
        }

        void Update();

        mat4 GetProjectionMatrix() const;
        mat4 GetViewMatrix() const;
        mat4 GetViewMatrixNoTranslate() const;
        vec3 GetDirection() const;
        vec3 GetPosition() const;

    public:
        //camera control
        void InitControl();
        void MoveForward();
        void MoveBackward();
        void MoveLeft();
        void MoveRight();
        void RotateStart();
        void RotateEnd();

    private:
        mat4 UpdateRotation();
        bool m_updateRotation;
        double m_cursorX, m_cursorY;

        vec3 m_center;
        vec3 m_eye;
        vec3 m_up;

        vec3 m_moveVec;
        vec3 m_rotateVec;

        SScreenInfo m_screenInfo = SScreenInfo(radians(45.f), 1024.0f / 768.0f, 0.1f, 500.0f);
    };
}