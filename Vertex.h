//
// Created by volts on 2026-11-11.
//

#ifndef CAELUSENGINE_VERTEX_H
#define CAELUSENGINE_VERTEX_H
#include "Math/Math.h"

namespace Caelus {
    struct Vertex {
        Math::Vector3 position;
    };

    static_assert(sizeof(Vertex) == 12, "Vertex must be tightly packed for the vertex input layout");
}

#endif //CAELUSENGINE_VERTEX_H
