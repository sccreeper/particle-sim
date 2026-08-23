#pragma once

#include <cassert>
#include <cstdint>

namespace utils {

    const uint8_t cTopLeft      = 0b10000000;
    const uint8_t cTopCentre    = 0b01000000;
    const uint8_t cTopRight     = 0b00100000;
    const uint8_t cLeft         = 0b00010000;
    const uint8_t cRight        = 0b00001000;
    const uint8_t cBottomLeft   = 0b00000100;
    const uint8_t cBottomCentre = 0b00000010;
    const uint8_t cBottomRight  = 0b00000001;
    const uint8_t cCentre       = 0b0;

    uint8_t getCheckCode(const int8_t displX, const int8_t displY);

    inline uint64_t idxToX(uint64_t idx, uint64_t width) { return idx % width; };
    inline uint64_t idxToY(uint64_t idx, uint64_t width) { return idx / width; };

    inline uint64_t xyToIdx(uint64_t x, uint64_t y, uint64_t width) { return (y * width) + x; };

    inline uint32_t linearlyInterpolateColour(double min, double max, uint32_t minColour, uint32_t maxColour,
                                              double value) {
        assert(max > min);
        assert(value <= max && value >= min);

        double lerpAmount = (value - min) / (max - min);

        uint8_t a_r = (minColour >> 24) & 0xFF, a_g = (minColour >> 16) & 0xFF, a_b = (minColour >> 8) & 0xFF,
                a_a = minColour & 0xFF;
        uint8_t b_r = (maxColour >> 24) & 0xFF, b_g = (maxColour >> 16) & 0xFF, b_b = (maxColour >> 8) & 0xFF,
                b_a = maxColour & 0xFF;

        uint8_t out_r = static_cast<uint8_t>(a_r + ((b_r - a_r) * lerpAmount));
        uint8_t out_g = static_cast<uint8_t>(a_g + ((b_g - a_g) * lerpAmount));
        uint8_t out_b = static_cast<uint8_t>(a_b + ((b_b - a_b) * lerpAmount));
        uint8_t out_a = static_cast<uint8_t>(a_a + ((b_a - a_a) * lerpAmount));

        uint32_t result = 0;
        result |= static_cast<uint32_t>(out_r) << 24;
        result |= static_cast<uint32_t>(out_g) << 16;
        result |= static_cast<uint32_t>(out_b) << 8;
        result |= static_cast<uint32_t>(out_a);

        return result;
    };

    namespace shapes {}

} // namespace utils