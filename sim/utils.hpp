#pragma once

#include <cstdint>

namespace utils {

    const uint8_t cTopLeft = 0b10000000;
    const uint8_t cTopCentre = 0b01000000;
    const uint8_t cTopRight = 0b00100000;
    const uint8_t cLeft = 0b00010000;
    const uint8_t cRight = 0b00001000;
    const uint8_t cBottomLeft = 0b00000100;
    const uint8_t cBottomCentre = 0b00000010;
    const uint8_t cBottomRight = 0b00000001;
    const uint8_t cCentre = 0b0;

    uint8_t getCheckCode(const int8_t displX, const int8_t displY);

    inline uint64_t idxToX(uint64_t idx, uint64_t width) {
        return idx % width;
    };
    inline uint64_t idxToY(uint64_t idx, uint64_t width) {
        return idx / width;
    };

    inline uint64_t xyToIdx(uint64_t x, uint64_t y, uint64_t width) {
        return (y * width) + x;
    };

    namespace shapes {

        

    }

}