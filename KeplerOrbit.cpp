//
// Created by volts on 2026-11-12.
//

#include "KeplerOrbit.h"

#include <numbers>

namespace Caelus
{
    Math::WorldPos GetPositionFromOrbit(const KeplerData& kd, double parentGravParam, double time)
    {
        auto n = sqrt(parentGravParam / (kd.semiMajorAxis * kd.semiMajorAxis * kd.semiMajorAxis));
        auto meanAnomaly = kd.meanAnomalyAtEpoch + n * time;
        
        meanAnomaly = fmod(meanAnomaly, 2.0 * std::numbers::pi);
        
        double E = (kd.eccentricity > 0.8) ? std::numbers::pi : meanAnomaly;
        
        for (int i = 0; i < 10; i++)
        {
            double dE = (E - kd.eccentricity * sin(E) - meanAnomaly) / (1.0 - kd.eccentricity * cos(E));
            E -= dE;
            if (std::abs(dE) < 1e-12) break;
        }
        
        double x = kd.semiMajorAxis * (cos(E) -kd.eccentricity);
        double y = kd.semiMajorAxis * sqrt(1.0 - kd.eccentricity * kd.eccentricity) * sin(E);
        double z = 0;

        double x1 = x * cos(kd.argOfPeriapsis) - y * sin(kd.argOfPeriapsis);
        double y1 = x * sin(kd.argOfPeriapsis) + y * cos(kd.argOfPeriapsis);

        double y2 = y1 * cos(kd.inclination);
        double z2 = y1 * sin(kd.inclination);

        double xF = x1 * cos(kd.longOfAscendingNode) - y2 * sin(kd.longOfAscendingNode);
        double yF = x1 * sin(kd.longOfAscendingNode) + y2 * cos(kd.longOfAscendingNode);
        double zF = z2;

        return {xF, zF, -yF}; //? JPL uses weird coordinates dont question or maybe do im not the expert
    }
}
