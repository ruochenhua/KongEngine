﻿#pragma once
#include <GLM/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

#include "Render/RenderModule.hpp"

namespace Kong
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
            : m_center(center), m_front(normalize(eye - center)), m_up(up)
        {
            
        }

        void Update(double delta);

        mat4 GetProjectionMatrix() const;
        mat4 GetViewMatrix() const;
        vec3 GetDirection() const;
        vec3 GetPosition() const;
        vec2 GetNearFar() const;

        void SetPosition(const vec3& position);
        void InvertPitch();
    
    public:
        //camera control
        void InitControl();
        void MoveForward();
        void MoveBackward();
        void MoveLeft();
        void MoveRight();
        void MoveUp();
        void MoveDown();
        void RotateStart();
        void RotateEnd();

        float exposure { 0.3f };
        float move_speed { 5.f };
        
        SScreenInfo m_screenInfo{radians(45.f), 1024.0f / 768.0f, 0.1f, 5000.0f};
    private:
        void UpdateRotation(double delta);
        void OnPYRUpdated();   // 更新pitch yaw roll
        bool m_updateRotation {false};
        double m_cursorX { 512.0 };
        double m_cursorY { 384.0 };

        vec3 m_center;
        vec3 m_front;
        vec3 m_up;

        vec3 m_moveVec {0.0f,0.0f,0.0f};

        double m_yaw { 0.0 };
        double m_pitch { 0.0 };
        float rotate_speed { 20.f };

    };
}