#include <cstdint>
#include <string>
#include <array>

struct Material {
    std::string name;
    double meltingPoint;
    double boilingPoint;
    double density;
    int32_t colour;
    bool flammable;
    double ignitionPoint;
};

struct Particle {
    int16_t materialId;
    int32_t lifetime;
    std::array<float, 2> velocity;
    float temperature;
}; 