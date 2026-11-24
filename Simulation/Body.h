//
// Created by volts on 2026-11-12.
//

#ifndef CAELUSENGINE_BODY_H
#define CAELUSENGINE_BODY_H
#include <optional>
#include <string>

#include "../Renderer/Atmosphere.h"
#include "../Math/Math.h"
#include "KeplerOrbit.h"

namespace Caelus {
    struct Body {
        static constexpr int noParent = -1;

        std::string name;
        int parent = noParent;
        Math::WorldPos localPosition;
        Math::WorldPos worldPosition;
        float radius = 1;
        bool emissive = false;
        std::optional<KeplerData> orbit;
        std::optional<double> gravParam;
        std::vector<Math::WorldPos> orbitPoints;
        std::string texturePath;
        int textureIndex = noParent;
        double rotationalPeriod = 0;
        double axialTilt = 0;
        std::optional<AtmosphereParams> atmosphereParams;
        int atmosphereResourseIndex;
    };
} // Caelus

#endif //CAELUSENGINE_BODY_H
