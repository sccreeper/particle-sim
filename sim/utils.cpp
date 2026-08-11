#include "utils.hpp"
#include <cstdint>
#include <array>
#include <cassert>

// (-1, -1) (0,-1)  (1, -1)
// (-1, 0)  (0, 0)  (1, 0)
// (-1, 1)  (0, 1)  (1, 1)

uint8_t utils::getCheckCode(const int8_t displX, const int8_t displY)
{
    assert((displX == 0 || displX == -1 || displX == 1) && (displY == 0 || displY == -1 || displY == 1) && "Displacements must be 0, -1, or 1");

    static constexpr std::array<uint8_t, 9> table = {
        cTopLeft, cTopCentre, cTopRight,
        cLeft, cCentre, cRight,
        cBottomLeft, cBottomCentre, cBottomRight};

    const uint8_t row = static_cast<uint8_t>(displY + 1);
    const uint8_t col = static_cast<uint8_t>(displX + 1);

    return table[(row * 3) + col];
}