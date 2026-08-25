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
        [[nodiscard]] Math::Matrix4x4 GetView() const;
        [[nodiscard]] Math::Vector3& GetPosition() {return m_position;} ;
        [[nodiscard]] float& GetPitch() {return m_pitch;} ;
        [[nodiscard]] float& GetYaw() {return m_yaw;} ;
    private:
        Math::Vector3 m_position{};
        float m_pitch = 0, m_yaw = 0, m_fov = 1.0472f;
    };
}

#endif //CAELUSENGINE_CAMERA_H
