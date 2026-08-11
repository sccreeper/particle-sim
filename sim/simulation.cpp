#include "simulation.hpp"
#include "materials.hpp"
#include "utils.hpp"
#include <utility>
#include <random>

Simulation::Simulation(size_t width, size_t height)
{
    this->materialRegistry = Registry<Material>{};
    rng = std::mt19937_64{std::random_device{}()};

    materialRegistry.registerItem({
        .name = "Sand",
        .meltingPoint = ZERO_DEGREES_C + 1713.0,
        .boilingPoint = ZERO_DEGREES_C + 2950.0,
        .mass = 1.92,
        .flammable = false,
        .ignitionPoint = -1,
        .hasStructure = false,
        .colour = 0xdcc472ff,
    });

    materialRegistry.registerItem({
        .name = "Granite",
        .meltingPoint = ZERO_DEGREES_C + 1260.0,
        .boilingPoint = ZERO_DEGREES_C + 2700.0,
        .mass = 1.75,
        .flammable = false,
        .ignitionPoint = -1,
        .hasStructure = true,
        .colour = 0xb77257ff,
    });

    materialRegistry.registerItem({
        .name = "Water",
        .meltingPoint = ZERO_DEGREES_C,
        .boilingPoint = ZERO_DEGREES_C + 100.0,
        .mass = 1.0,
        .flammable = false,
        .ignitionPoint = -1,
        .hasStructure = true,
        .colour = 0x2389daff,
    });

    materialRegistry.registerItem({
        .name = "Oil",
        .meltingPoint = ZERO_DEGREES_C - 30.0,
        .boilingPoint = ZERO_DEGREES_C + 250.0,
        .mass = 0.79,
        .flammable = true,
        .ignitionPoint = ZERO_DEGREES_C + 70.0,
        .hasStructure = true,
        .colour = 0x631007ff,
    });

    this->width = width;
    this->height = height;

    this->particles.resize(width * height, Particle{.occupied = false});
}

void Simulation::tick()
{

    for (size_t i = 0; i < particles.size(); i++)
    {

        if (particles[i].occupied)
        {

            int64_t x = utils::idxToX(i, this->width);
            int64_t y = utils::idxToY(i, this->width);

            switch (particles[i].state)
            {
            case Liquid:
            {
                int deltaX = 0;
                bool moveX = this->coinFlip();

                if (moveX)
                {
                    bool direction = this->coinFlip();
                    deltaX = direction ? 1 : -1;

                    if (x + deltaX < 0 || x + deltaX >= width)
                    {
                        deltaX *= -1;
                    }
                }

                int deltaY = 0;
                if (y + 1 < height)
                    deltaY = +1;

                // Always prioritise y down
                if (canSwap(x, y + deltaY, i))
                {
                    std::swap(particles[i], particles[utils::xyToIdx(x, y + deltaY, this->width)]);
                } else if (canSwap(x + deltaX, y + deltaY, i))
                {
                    std::swap(particles[i], particles[utils::xyToIdx(x + deltaX, y + deltaY, this->width)]);
                }

                break;
            }
            case Solid:
            {
                if (!materialRegistry.getItem(particles[i].materialId)->hasStructure)
                {
                    int deltaY = 0;
                    if (y + 1 < height)
                        deltaY = +1;

                    if (canSwap(x, y + deltaY, i))
                    {
                        std::swap(particles[i], particles[utils::xyToIdx(x, y + deltaY, this->width)]);
                        break;
                    }

                    deltaY = 0;
                    if (y + 2 < height)
                        deltaY = +2;
                    if (deltaY == 0)
                        break;

                    uint8_t possibleSides = 0b00;
                    if (!(x + -1 < 0) && canSwap(x - 1, y + deltaY, i))
                        possibleSides |= 0b10;
                    if (x + 1 < this->width && canSwap(x + 1, y + deltaY, i))
                        possibleSides |= 0b01;

                    if (coinFlip() && possibleSides & 0b10)
                    {
                        std::swap(particles[i], particles[utils::xyToIdx(x + -1, y + deltaY, this->width)]);
                    }
                    else if (possibleSides & 0b01)
                    {
                        std::swap(particles[i], particles[utils::xyToIdx(x + 1, y + deltaY, this->width)]);
                    }
                }
                break;
            }
            case Gas:
            {
                bool leftRight = coinFlip();
                bool topBottom = coinFlip();

                int deltaX = leftRight ? 1 : -1;
                int deltaY = topBottom ? 1 : -1;

                if (canSwap(x + deltaX, y + deltaY, i))
                {
                    std::swap(particles[i], particles[utils::xyToIdx(x + deltaX, y + deltaY, width)]);
                }

                break;
            }
            default:
                break;
            }
        }
    }
}

const Particle &Simulation::operator[](size_t idx) const
{
    return particles.at(static_cast<size_t>(idx));
}

Particle &Simulation::operator[](size_t idx)
{
    return const_cast<Particle &>(std::as_const(*this)[idx]);
}

bool Simulation::coinFlip()
{
    if (bitsLeft == 0)
    {
        randomBitBuffer = rng();
        bitsLeft = 64;
    }

    bool result = randomBitBuffer & 1;
    --bitsLeft;
    randomBitBuffer >>= 1;

    return result;
}

// Swapping logic for fluids (gas, liquid), and non-structural solids, e.g. sand
bool Simulation::canSwap(int x, int y, int idx)
{

    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return false;
    }

    auto ref = &particles[utils::xyToIdx(x, y, this->width)];

    if (!ref->occupied)
    {
        return true;
    }
    else if (
        ref->occupied && ref->state == Liquid &&
        particles[idx].state == Solid &&
        !materialRegistry.getItem(particles[idx].materialId)->hasStructure &&
        materialRegistry.getItem(ref->materialId)->mass < materialRegistry.getItem(particles[idx].materialId)->mass) // non-structural solids sinking in fluids
    {
        return true;
    }
    else if (
        ref->occupied &&
        (ref->state == Liquid || ref->state == Gas) &&
        (particles[idx].state == Liquid || particles[idx].state == Gas) &&
        materialRegistry.getItem(ref->materialId)->mass < materialRegistry.getItem(particles[idx].materialId)->mass // fluids sinking in fluids
    )
    {
        return true;
    }

    return false;
}