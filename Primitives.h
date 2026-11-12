//
// Created by volts on 2026-11-12.
//

#ifndef CAELUSENGINE_PRIMITIVES_H
#define CAELUSENGINE_PRIMITIVES_H
#include <vector>

#include "Vertex.h"

namespace Caelus
{
    struct MeshData
    {
        std::vector<Caelus::Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };
    MeshData createCubeSphere(int resolution);
} // Caelus

#endif //CAELUSENGINE_PRIMITIVES_H
