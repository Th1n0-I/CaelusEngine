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
        [[nodiscard]] Math::WorldPos& GetPosition() {return m_position;} ;
        [[nodiscard]] float& GetPitch() {return m_pitch;} ;
        [[nodiscard]] float& GetYaw() {return m_yaw;} ;
        void LookAround(float deltaX, float deltaY);
        Math::Vector3 GetForward() const;
        void Follow(const Math::WorldPos& target);
        double& GetDistance() {return m_distance;}
        void SetDistance(const double distance) {m_distance = distance;}
    private:
        Math::WorldPos m_position{};
        float m_pitch = 0, m_yaw = 0, m_fov = 1.0472f;
        double m_distance = 3.0e7;
    };
}

#endif //CAELUSENGINE_CAMERA_H
