//
// Created by volts on 2026-11-12.
//

#include "Primitives.h"

namespace Caelus {
    MeshData createCubeSphere(int resolution) {
        MeshData md{};
        const int R = resolution + 1;

        std::vector<int> indexSpots(R * R * R, -1);

        for (int x = 0; x <= resolution; x++) {
            float posX = static_cast<float>(x) / static_cast<float>(resolution) - 0.5f;
            bool xOnEdge = x == 0 || x == resolution;
            for (int y = 0; y <= resolution; y++) {
                float posY = static_cast<float>(y) / static_cast<float>(resolution) - 0.5f;
                bool yOnEdge = y == 0 || y == resolution;
                for (int z = 0; z <= resolution; z++) {
                    float posZ = static_cast<float>(z) / static_cast<float>(resolution) - 0.5f;
                    bool zOnEdge = z == 0 || z == resolution;
                    if (!xOnEdge && !yOnEdge && !zOnEdge) continue;
                    int mapIndex = (x * R + y) * R + z;
                    indexSpots[mapIndex] = md.Vertices.size();
                    md.Vertices.push_back({Math::normalize(Math::Vector3{posX, posY, posZ})});
                }
            }
        }

        md.Indices.reserve(36 * resolution * resolution);

        auto vert = [&](int x, int y, int z) { return static_cast<uint32_t>(indexSpots[(x * R + y) * R + z]); };

        auto addQuad = [&](uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool flip) {
            if (!flip) md.Indices.insert(md.Indices.end(), {a, b, c, a, c, d});
            else md.Indices.insert(md.Indices.end(), {a, c, b, a, d, c});
        };

        for (int i = 0; i < resolution; i++) {
            for (int j = 0; j < resolution; j++) {
                for (int side = 0; side <= resolution; side += resolution) {
                    bool flip = (side == resolution);
                    addQuad(vert(side, i, j), vert(side, i + 1, j), vert(side, i + 1, j + 1), vert(side, i, j + 1),
                            flip);
                    addQuad(vert(j, side, i), vert(j, side, i + 1), vert(j + 1, side, i + 1), vert(j + 1, side, i),
                            flip);
                    addQuad(vert(i, j, side), vert(i + 1, j, side), vert(i + 1, j + 1, side), vert(i, j + 1, side),
                            flip);
                }
            }
        }
        return md;
    }
} // Caelus