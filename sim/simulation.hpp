#pragma once

#include <cstdint>
#include <random>
#include <utility>
#include <vector>

#include <stddef.h>

#include "materials.hpp"
#include "registry.hpp"
#include "utils.hpp"

namespace sim {

    const double ZERO_DEGREES_C = 273.15;

    class Simulation {

      public:
        Simulation(size_t width, size_t height);
        ~Simulation() = default;
        void tick();
        void updatePixelBuffer();
        const uint8_t *getPixelBuffer();
        mat::Particle &operator[](size_t idx);
        const mat::Particle &operator[](size_t idx) const;

        float ambientTemperature = ZERO_DEGREES_C + 25;

        bool coinFlip();
        bool canSwap(int x, int y, int idx);

        int drawRectangle(int x, int y, int width, int height, const int16_t materialId, bool fill = true,
                          bool replace = false);
        int drawCircle(int x, int y, int radius, const int16_t materialId, bool fill = true,
                       bool replace = false);
        int drawLine(int x0, int y0, int x1, int y1, const int16_t materialId, bool fill = true,
                     bool replace = false);

        void printDebugInfo();

        Registry<mat::Material> materialRegistry;

      private:
        std::vector<mat::Particle> particles;
        std::vector<bool> movedThisTick; // separate array for the
                                         // purposes of fast clearing
        std::vector<uint8_t> pixelBuffer;
        size_t width;
        size_t height;
        bool leftToRight = false;

        inline void move(size_t srcIdx, int64_t destX, int64_t destY) {
            std::swap(particles[srcIdx], particles[utils::xyToIdx(destX, destY, width)]);
            movedThisTick[utils::xyToIdx(destX, destY, width)] = true;
        };

        uint64_t randomBitBuffer;
        std::mt19937_64 rng;
        int bitsLeft = 0;
    };

}; // namespace sim