//
// Created by volts on 2026-08-24.
//

#ifndef CAELUSENGINE_CAMERA_H
#define CAELUSENGINE_CAMERA_H

#include "Math/Math.h"

namespace Caelus {
    class Camera {
    public:
        [[nodiscard]] Math::Matrix4x4 GetPerspective(float aspect, float nearZ, float farZ) const;
    private:
        Math::Vector3 m_position{};
        float m_pitch = 0, m_yaw = 0, m_fov = 1.0472f;
    };
}

#endif //CAELUSENGINE_CAMERA_H
