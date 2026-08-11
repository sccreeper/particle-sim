#pragma once

#include <vector>
#include <cstdint>
#include "materials.hpp"
#include "registry.hpp"
#include <random>

const double ZERO_DEGREES_C = 273.15;

class Simulation
{

    Simulation(size_t width, size_t height);
    ~Simulation() = default;

public:
    void tick();
    Particle &operator[](size_t idx);
    const Particle &operator[](size_t idx) const;

    double ambientTemperature = ZERO_DEGREES_C + 25;

    bool coinFlip();
    bool canSwap(int x, int y, int idx);

private:
    std::vector<Particle> particles;
    Registry<Material> materialRegistry;
    size_t width;
    size_t height;

    uint64_t randomBitBuffer;
    std::mt19937_64 rng;
    int bitsLeft = 0;
};