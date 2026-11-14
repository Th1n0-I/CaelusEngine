//
// Created by volts on 2026-11-14.
//


//! Body values were gathered and written by AI cuz I don't wanna type allat.
#include "SolarSystem.h"

#include <numbers>

namespace Caelus {
    std::vector<Body> createSolarSystem() {
        // unit helpers: lets us type values exactly as the JPL tables give them
        constexpr double AU = 1.495978707e11; // meters, exact by IAU definition
        constexpr double deg = std::numbers::pi / 180.0; // degrees -> radians
        constexpr double km = 1000.0; // km -> meters
        constexpr double km3 = 1.0e9; // km^3/s^2 -> m^3/s^2

        // ---- index 0
        Caelus::Body sun;
        sun.name = "Sun";
        sun.parent = Caelus::Body::noParent;
        sun.radius = 695700.0 * km;
        sun.emissive = true;
        sun.gravParam = 1.32712440018e11 * km3;
        sun.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/8k_sun.jpg";

        // ---- index 1
        Caelus::Body mercury;
        mercury.name = "Mercury";
        mercury.parent = 0;
        mercury.radius = 2439.7 * km;
        mercury.gravParam = 22032.0 * km3;
        mercury.orbit = Caelus::KeplerData{
            0.38709927 * AU,
            0.20563593,
            7.00497902 * deg,
            (252.25032350 - 77.45779628) * deg, // M0 = L - long.peri
            (77.45779628 - 48.33076593) * deg, // w  = long.peri - long.node
            48.33076593 * deg
        };
        mercury.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/8k_mercury.jpg";

        // ---- index 2
        Caelus::Body venus;
        venus.name = "Venus";
        venus.parent = 0;
        venus.radius = 6051.8 * km;
        venus.gravParam = 324859.0 * km3;
        venus.orbit = Caelus::KeplerData{
            0.72333566 * AU,
            0.00677672,
            3.39467605 * deg,
            (181.97909950 - 131.60246718) * deg,
            (131.60246718 - 76.67984255) * deg,
            76.67984255 * deg
        };
        venus.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/4k_venus_atmosphere.jpg";

        // ---- index 3
        Caelus::Body earth;
        earth.name = "Earth";
        earth.parent = 0;
        earth.radius = 6371.0 * km;
        earth.gravParam = 398600.4418 * km3;
        earth.orbit = Caelus::KeplerData{
            1.00000261 * AU,
            0.01671123,
            -0.00001531 * deg,
            (100.46457166 - 102.93768193) * deg,
            (102.93768193 - 0.0) * deg,
            0.0 * deg
        };
        earth.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/8k_earth_daymap.jpg";

        // ---- index 5
        Caelus::Body mars;
        mars.name = "Mars";
        mars.parent = 0;
        mars.radius = 3389.5 * km;
        mars.gravParam = 42828.37 * km3;
        mars.orbit = Caelus::KeplerData{
            1.52371034 * AU,
            0.09339410,
            1.84969142 * deg,
            (-4.55343205 - -23.94362959) * deg,
            (-23.94362959 - 49.55953891) * deg,
            49.55953891 * deg
        };
        mars.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/8k_mars.jpg";

        // ---- index 8
        Caelus::Body jupiter;
        jupiter.name = "Jupiter";
        jupiter.parent = 0;
        jupiter.radius = 69911.0 * km;
        jupiter.gravParam = 126686534.0 * km3;
        jupiter.orbit = Caelus::KeplerData{
            5.20288700 * AU,
            0.04838624,
            1.30439695 * deg,
            (34.39644051 - 14.72847983) * deg,
            (14.72847983 - 100.47390909) * deg,
            100.47390909 * deg
        };
        jupiter.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/8k_jupiter.jpg";

        // ---- index 13
        Caelus::Body saturn;
        saturn.name = "Saturn";
        saturn.parent = 0;
        saturn.radius = 58232.0 * km;
        saturn.gravParam = 37931187.0 * km3;
        saturn.orbit = Caelus::KeplerData{
            9.53667594 * AU,
            0.05386179,
            2.48599187 * deg,
            (49.95424423 - 92.59887831) * deg,
            (92.59887831 - 113.66242448) * deg,
            113.66242448 * deg
        };
        saturn.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/8k_saturn.jpg";

        // ---- index 21
        Caelus::Body uranus;
        uranus.name = "Uranus";
        uranus.parent = 0;
        uranus.radius = 25362.0 * km;
        uranus.gravParam = 5793939.0 * km3;
        uranus.orbit = Caelus::KeplerData{
            19.18916464 * AU,
            0.04725744,
            0.77263783 * deg,
            (313.23810451 - 170.95427630) * deg,
            (170.95427630 - 74.01692503) * deg,
            74.01692503 * deg
        };
        uranus.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/2k_uranus.jpg";

        // ---- index 27
        Caelus::Body neptune;
        neptune.name = "Neptune";
        neptune.parent = 0;
        neptune.radius = 24622.0 * km;
        neptune.gravParam = 6836529.0 * km3;
        neptune.orbit = Caelus::KeplerData{
            30.06992276 * AU,
            0.00859048,
            1.77004347 * deg,
            (-55.12002969 - 44.96476227) * deg,
            (44.96476227 - 131.78422574) * deg,
            131.78422574 * deg
        };
        neptune.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/2k_neptune.jpg";

        // ---- index 4
        Caelus::Body moon;
        moon.name = "Moon";
        moon.parent = 3;
        moon.radius = 1737.4 * km;
        moon.gravParam = 4902.8 * km3;
        moon.orbit = Caelus::KeplerData{384400.0 * km, 0.0549, 5.145 * deg, 0.0, 0.0, 0.0};
        moon.texturePath = "C:/Users/volts/Documents/SolarSystemTextures/8k_moon.jpg";


        // ---- index 6
        Caelus::Body phobos;
        phobos.name = "Phobos";
        phobos.parent = 5;
        phobos.radius = 11.1 * km;
        phobos.gravParam = 0.000709 * km3;
        phobos.orbit = Caelus::KeplerData{9376.0 * km, 0.0151, 1.08 * deg, 0.0, 0.0, 0.0};

        // ---- index 7
        Caelus::Body deimos;
        deimos.name = "Deimos";
        deimos.parent = 5;
        deimos.radius = 6.2 * km;
        deimos.gravParam = 0.000096 * km3;
        deimos.orbit = Caelus::KeplerData{23463.0 * km, 0.0003, 1.79 * deg, 2.0, 0.0, 0.0};

        // ---- index 9
        Caelus::Body ioMoon;
        ioMoon.name = "Io";
        ioMoon.parent = 8;
        ioMoon.radius = 1821.6 * km;
        ioMoon.gravParam = 5959.9 * km3;
        ioMoon.orbit = Caelus::KeplerData{421800.0 * km, 0.0041, 0.04 * deg, 0.0, 0.0, 0.0};

        // ---- index 10
        Caelus::Body europa;
        europa.name = "Europa";
        europa.parent = 8;
        europa.radius = 1560.8 * km;
        europa.gravParam = 3202.7 * km3;
        europa.orbit = Caelus::KeplerData{671100.0 * km, 0.0094, 0.47 * deg, 1.6, 0.0, 0.0};

        // ---- index 11
        Caelus::Body ganymede;
        ganymede.name = "Ganymede";
        ganymede.parent = 8;
        ganymede.radius = 2631.2 * km;
        ganymede.gravParam = 9887.8 * km3;
        ganymede.orbit = Caelus::KeplerData{1070400.0 * km, 0.0013, 0.18 * deg, 3.1, 0.0, 0.0};

        // ---- index 12
        Caelus::Body callisto;
        callisto.name = "Callisto";
        callisto.parent = 8;
        callisto.radius = 2410.3 * km;
        callisto.gravParam = 7179.3 * km3;
        callisto.orbit = Caelus::KeplerData{1882700.0 * km, 0.0074, 0.19 * deg, 4.7, 0.0, 0.0};

        // ---- index 14
        Caelus::Body mimas;
        mimas.name = "Mimas";
        mimas.parent = 13;
        mimas.radius = 198.2 * km;
        mimas.gravParam = 2.50 * km3;
        mimas.orbit = Caelus::KeplerData{185540.0 * km, 0.0196, 1.57 * deg, 0.0, 0.0, 0.0};

        // ---- index 15
        Caelus::Body enceladus;
        enceladus.name = "Enceladus";
        enceladus.parent = 13;
        enceladus.radius = 252.1 * km;
        enceladus.gravParam = 7.21 * km3;
        enceladus.orbit = Caelus::KeplerData{238040.0 * km, 0.0047, 0.01 * deg, 0.9, 0.0, 0.0};

        // ---- index 16
        Caelus::Body tethys;
        tethys.name = "Tethys";
        tethys.parent = 13;
        tethys.radius = 531.1 * km;
        tethys.gravParam = 41.2 * km3;
        tethys.orbit = Caelus::KeplerData{294670.0 * km, 0.0001, 1.09 * deg, 1.8, 0.0, 0.0};

        // ---- index 17
        Caelus::Body dione;
        dione.name = "Dione";
        dione.parent = 13;
        dione.radius = 561.4 * km;
        dione.gravParam = 73.1 * km3;
        dione.orbit = Caelus::KeplerData{377420.0 * km, 0.0022, 0.03 * deg, 2.7, 0.0, 0.0};

        // ---- index 18
        Caelus::Body rhea;
        rhea.name = "Rhea";
        rhea.parent = 13;
        rhea.radius = 763.8 * km;
        rhea.gravParam = 153.9 * km3;
        rhea.orbit = Caelus::KeplerData{527070.0 * km, 0.0010, 0.35 * deg, 3.6, 0.0, 0.0};

        // ---- index 19
        Caelus::Body titan;
        titan.name = "Titan";
        titan.parent = 13;
        titan.radius = 2574.7 * km;
        titan.gravParam = 8978.1 * km3;
        titan.orbit = Caelus::KeplerData{1221870.0 * km, 0.0288, 0.31 * deg, 4.5, 0.0, 0.0};

        // ---- index 20
        Caelus::Body iapetus;
        iapetus.name = "Iapetus";
        iapetus.parent = 13;
        iapetus.radius = 734.5 * km;
        iapetus.gravParam = 120.5 * km3;
        iapetus.orbit = Caelus::KeplerData{3560840.0 * km, 0.0283, 15.5 * deg, 5.4, 0.0, 0.0};

        // ---- index 22
        Caelus::Body miranda;
        miranda.name = "Miranda";
        miranda.parent = 21;
        miranda.radius = 235.8 * km;
        miranda.gravParam = 4.4 * km3;
        miranda.orbit = Caelus::KeplerData{129900.0 * km, 0.0013, 4.34 * deg, 0.0, 0.0, 0.0};

        // ---- index 23
        Caelus::Body ariel;
        ariel.name = "Ariel";
        ariel.parent = 21;
        ariel.radius = 578.9 * km;
        ariel.gravParam = 83.5 * km3;
        ariel.orbit = Caelus::KeplerData{190900.0 * km, 0.0012, 0.04 * deg, 1.3, 0.0, 0.0};

        // ---- index 24
        Caelus::Body umbriel;
        umbriel.name = "Umbriel";
        umbriel.parent = 21;
        umbriel.radius = 584.7 * km;
        umbriel.gravParam = 85.1 * km3;
        umbriel.orbit = Caelus::KeplerData{266000.0 * km, 0.0039, 0.13 * deg, 2.5, 0.0, 0.0};

        // ---- index 25
        Caelus::Body titania;
        titania.name = "Titania";
        titania.parent = 21;
        titania.radius = 788.9 * km;
        titania.gravParam = 226.9 * km3;
        titania.orbit = Caelus::KeplerData{436300.0 * km, 0.0011, 0.08 * deg, 3.8, 0.0, 0.0};

        // ---- index 26
        Caelus::Body oberon;
        oberon.name = "Oberon";
        oberon.parent = 21;
        oberon.radius = 761.4 * km;
        oberon.gravParam = 205.3 * km3;
        oberon.orbit = Caelus::KeplerData{583500.0 * km, 0.0014, 0.07 * deg, 5.0, 0.0, 0.0};

        // ---- index 28
        Caelus::Body triton;
        triton.name = "Triton";
        triton.parent = 27;
        triton.radius = 1353.4 * km;
        triton.gravParam = 1428.5 * km3;
        triton.orbit = Caelus::KeplerData{354759.0 * km, 0.000016, 156.865 * deg, 0.0, 0.0, 0.0};
        // >90 deg = retrograde

        std::vector<Caelus::Body> bodies{
            sun, mercury, venus, earth, moon, mars, phobos, deimos,
            jupiter, ioMoon, europa, ganymede, callisto,
            saturn, mimas, enceladus, tethys, dione, rhea, titan, iapetus,
            uranus, miranda, ariel, umbriel, titania, oberon,
            neptune, triton
        };

        for (auto& body : bodies) {
            if (body.orbit) body.orbitPoints = Caelus::GetOrbitPoints(body.orbit.value(), orbitLineResolution,
                                                                      bodies[body.parent].gravParam.value());
        }

        return bodies;
    }
} // Caelus
