#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace mat {

    enum State { Solid, Liquid, Gas };

    struct Material {
        std::string name;
        float       meltingPoint;
        float       boilingPoint;
        float       mass;
        bool        flammable;
        float       ignitionPoint;
        bool        hasStructure;
        uint32_t    colour;
    };

    State decideState(const Material &mat, float temp);

    struct Particle {
        int16_t              materialId;
        int32_t              lifetime;
        std::array<float, 2> velocity;
        float                temperature;
        State                state;
        uint8_t              temperatureInteractions;
        float                compression;

        bool occupied;
    };

} // namespace mat