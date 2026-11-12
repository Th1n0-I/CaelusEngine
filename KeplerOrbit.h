//
// Created by volts on 2026-11-12.
//

#ifndef CAELUSENGINE_KEPLERORBIT_H
#define CAELUSENGINE_KEPLERORBIT_H
#include "Math/Math.h"


namespace Caelus{
    struct KeplerData
    {
        double semiMajorAxis = 0.0;
        double eccentricity = 0.0;
        double inclination = 0.0;
        double meanAnomalyAtEpoch = 0.0;
        double argOfPeriapsis = 0.0;
        double longOfAscendingNode = 0.0;
    };
    
    Math::WorldPos GetPositionFromOrbit(const KeplerData& kd, double parentGravParam, double time);
    
}



#endif //CAELUSENGINE_KEPLERORBIT_H
