//
// Created by volts on 2026-08-24.
//

#include "Camera.h"

using namespace Caelus::Math;

namespace Caelus {
    Matrix4x4 Camera::GetPerspective(const float aspect, const float nearZ, const float farZ) const {
        const float f = 1.0f / tanf(m_fov * 0.5f);

        Matrix4x4 r{};

        r.m[0][0] = f / aspect;
        r.m[1][1] = -f;
        //? Maps near to 1 and far to 0.
        r.m[2][2] = 0.0f;
        r.m[3][2] = nearZ;
        r.m[2][3] = -1.0f;
        return r;
    }

    Matrix4x4 Camera::GetView() const {
        return ::rotateX(-m_pitch) *
            ::rotateY(-m_yaw);
    }

    void Camera::LookAround(float deltaX, float deltaY) {
        m_pitch += deltaX;
        m_yaw += deltaY;
    }

    Math::Vector3 Camera::GetForward() const {
        Matrix4x4 v = GetView();
        Vector3 forward = {-v.m[0][2], -v.m[1][2], -v.m[2][2]};

        return forward;
    }

    void Camera::Follow(const Math::WorldPos& target) {
        auto forward = GetForward();
        m_position = target - WorldPos{forward.x, forward.y, forward.z} * m_distance;
    }
}
