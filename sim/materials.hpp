#pragma once

#include <cstdint>
#include <string>
#include <array>

enum State {
    Solid,
    Liquid,
    Gas
};

struct Material {
    std::string name;
    double meltingPoint;
    double boilingPoint;
    double mass;
    bool flammable;
    double ignitionPoint;
    bool hasStructure;
    uint32_t colour;
};

struct Particle {
    int16_t materialId;
    int32_t lifetime;
    std::array<float, 2> velocity;
    float temperature;
    State state;
    uint8_t temperatureInteractions;

    bool occupied;
}; 