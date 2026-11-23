//
// Created by volts on 2026-11-11.
//

#ifndef CAELUSENGINE_MESH_H
#define CAELUSENGINE_MESH_H
#include "Buffer.h"

namespace Caelus {
    struct Mesh {
        Buffer vertexBuffer;
        Buffer indexBuffer;
        uint32_t indexCount;
    };
}

#endif //CAELUSENGINE_MESH_H
