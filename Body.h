//
// Created by volts on 2026-11-12.
//

#ifndef CAELUSENGINE_BODY_H
#define CAELUSENGINE_BODY_H
#include <optional>
#include <string>

#include "Math/Math.h"
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
    };
} // Caelus

#endif //CAELUSENGINE_BODY_H
