//
// Created by volts on 2026-11-12.
//

#include "KeplerOrbit.h"

#include <numbers>

namespace Caelus
{
    Math::WorldPos GetPositionFromOrbit(const KeplerData& kd, double parentGravParam, double time)
    {
        auto n = sqrt(parentGravParam / (kd.semiMajorAxis * kd.semiMajorAxis));
        auto meanAnomaly = kd.meanAnomalyAtEpoch + n * time;
        
        meanAnomaly = fmod(meanAnomaly, 2.0 * std::numbers::pi);
        
        double E = (kd.eccentricity > 0.8) ? std::numbers::pi : meanAnomaly;
        
        for (int i = 0; i < 10; i++)
        {
            double dE = (E - kd.eccentricity * sin(E) - meanAnomaly) / (1.0 - kd.eccentricity * cos(E));
            E -= dE;
            if (abs(dE) < 1e-12) break;
        }
        
        auto x = kd.semiMajorAxis * (cos(E - kd.eccentricity));
        auto y = kd.semiMajorAxis * sqrt()
    }
}
