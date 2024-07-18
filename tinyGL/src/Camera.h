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
            : m_center(center), m_front(normalize(eye - center)), m_up(up), m_moveVec(vec3(0, 0, 0))
            , m_updateRotation(false), m_cursorX(0.0), m_cursorY(0.0)
        {
            
        }

        void Update(double delta);

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
        void UpdateRotation(double delta);
        bool m_updateRotation;
        double m_cursorX = 512.0;
        double m_cursorY = 384.0;

        vec3 m_center;
        vec3 m_front;
        vec3 m_up;

        vec3 m_moveVec;

        float m_yaw = 0.0;
        float m_pitch = 0.0;
        float rotate_speed = 20.f;
        float move_speed = 50.f

        SScreenInfo m_screenInfo = SScreenInfo(radians(45.f), 1024.0f / 768.0f, 0.1f, 500.0f);
    };
}