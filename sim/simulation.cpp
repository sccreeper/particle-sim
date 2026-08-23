#include "simulation.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <format>
#include <limits>
#include <mutex>
#include <random>
#include <string>
#include <utility>

#include "logging.hpp"
#include "materials.hpp"
#include "utils.hpp"

namespace sim {

    Simulation::Simulation(size_t width, size_t height, int64_t speed) {
        this->materialRegistry = Registry<mat::Material>{};
        rng                    = std::mt19937_64{std::random_device{}()};

        materialRegistry.registerItem({
            .name          = "Sand",
            .meltingPoint  = ZERO_DEGREES_C + 1713.0,
            .boilingPoint  = ZERO_DEGREES_C + 2950.0,
            .mass          = 1.92,
            .flammable     = false,
            .ignitionPoint = -1,
            .hasStructure  = false,
            .colour        = 0xdcc472ff,
        });

        materialRegistry.registerItem({
            .name          = "Granite",
            .meltingPoint  = ZERO_DEGREES_C + 1260.0,
            .boilingPoint  = ZERO_DEGREES_C + 2700.0,
            .mass          = 1.75,
            .flammable     = false,
            .ignitionPoint = -1,
            .hasStructure  = true,
            .colour        = 0xb77257ff,
        });

        materialRegistry.registerItem({
            .name          = "Water",
            .meltingPoint  = ZERO_DEGREES_C,
            .boilingPoint  = ZERO_DEGREES_C + 100.0,
            .mass          = 1.0,
            .flammable     = false,
            .ignitionPoint = -1,
            .hasStructure  = true,
            .colour        = 0x2389daff,
        });

        materialRegistry.registerItem({
            .name          = "Oil",
            .meltingPoint  = ZERO_DEGREES_C - 30.0,
            .boilingPoint  = ZERO_DEGREES_C + 250.0,
            .mass          = 0.79,
            .flammable     = true,
            .ignitionPoint = ZERO_DEGREES_C + 70.0,
            .hasStructure  = true,
            .colour        = 0x631007ff,
        });

        this->width  = width;
        this->height = height;

        this->particles.resize(width * height,
                               mat::Particle{.temperature = ambientTemperature, .occupied = false});
        this->pixelBuffer.resize(width * height * 4, 0x000000FF);
        this->movedThisTick.resize(width * height, false);

        this->simSpeed.store(speed, std::memory_order_release);
    }

    Simulation::~Simulation() { stop(); }

    // Brief overview of the threading logic
    // See explanations for each method below
    //
    // The paused variable is a standard bool so it can be passed to the conditional variable. The CV checks
    // this first before going to sleep, and wakes when it is notified.
    // The other variables are atomic so they can be accessed without mutexes.
    // isThreadRunning tells us wether or not the thread should be running.
    // isPausedAtomic is a mirror of isPaused, so it can be read externally requiring mutexes, otherwise the
    // thread would be unnecessarily stalled.

    // Starts the simulation from scratch.
    void Simulation::start() {
        isThreadRunning.store(true, std::memory_order_release);
        simThread = std::thread([this] { runLoop(); });
    }

    // Pauses the thread, but doesn't stop it.
    void Simulation::pause() {
        std::lock_guard<std::mutex> lock(pausedMutex);
        isPaused = true;
        isPausedAtomic.store(isPaused, std::memory_order_release);
    }

    // Resumes the thread after pausing
    void Simulation::resume() {
        std::lock_guard<std::mutex> lock(pausedMutex);
        isPaused = false;
        isPausedAtomic.store(isPaused, std::memory_order_release);
        resumeNotifier.notify_one();
    }

    // Completely stops the thread, also called on destruction.
    void Simulation::stop() {
        std::lock_guard<std::mutex> lock(pausedMutex);
        isThreadRunning.store(false, std::memory_order_release);
        isPaused = false;
        isPausedAtomic.store(isPaused, std::memory_order_release);

        resumeNotifier.notify_one();
        if (simThread.joinable())
            simThread.join();
    }

    // The thread method
    // In this order we:
    // 1. Check that the thread is supposed to be running
    // 2. Lock the paused mutex for the purposes of the conditional_variable
    // 3. After we exit the conditional variable, break if the thread is no longer running
    // 4. Tick
    // 5. Sleep the thread for 500us
    void Simulation::runLoop() {
        while (isThreadRunning.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(pausedMutex);
            resumeNotifier.wait(lock, [this] { return !isPaused || !isThreadRunning; });

            if (!isThreadRunning.load(std::memory_order_acquire))
                break;

            lock.unlock();

            tick();

            std::this_thread::sleep_for(std::chrono::microseconds(simSpeed.load(std::memory_order_acquire)));
        }
    }

    bool Simulation::getIsPaused() { return isPausedAtomic.load(std::memory_order_acquire); }

    int64_t Simulation::getSimSpeed() { return simSpeed.load(std::memory_order_acquire); }

    void Simulation::setSimSpeed(int64_t val) {
        if (val < 1) {
            val = 1;
        }

        simSpeed.store(val, std::memory_order_release);
    }

    void Simulation::tick() {
        std::lock_guard<std::mutex> lock(particleAccessMutex);

        this->leftToRight = !(this->leftToRight);

        std::fill(movedThisTick.begin(), movedThisTick.end(), false);

        this->maxCompression = std::numeric_limits<float>::min();
        this->minCompression = std::numeric_limits<float>::max();

        // Firstly calculate compression for liquid particles
        for (size_t i = 0; i < particles.size(); i++) {
            if (particles[i].occupied && particles[i].state == mat::Liquid) {

                float compressionCounter = 0;

                uint64_t particleX = utils::idxToX(i, width);
                uint64_t particleY = utils::idxToY(i, width);

                do {
                    auto particleRef = &particles[utils::xyToIdx(particleX, particleY, width)];

                    if (particleRef->state == mat::Liquid) {
                        compressionCounter += materialRegistry.getItem(particleRef->materialId).mass;
                    } else {
                        break;
                    }
                    particleY--;
                } while (particleY >= 0);

                particles[i].compression = compressionCounter;
                if (particles[i].compression < minCompression) {
                    this->minCompression = particles[i].compression;
                } else if (particles[i].compression > maxCompression) {
                    this->maxCompression = particles[i].compression;
                }
            }
        }

        // Then move particles
        for (size_t row = 0; row < height; row++) {
            for (size_t col = 0; col < width; col++) {

                int64_t x = this->leftToRight ? col : (width - 1 - col);
                size_t  i = utils::xyToIdx(x, row, width);

                if (particles[i].occupied && !movedThisTick[i]) {

                    int64_t y = row;

                    switch (particles[i].state) {
                    case mat::Liquid: {

                        bool canGoDownLeft  = canSwap(x - 1, y + 1, i);
                        bool canGoDownRight = canSwap(x + 1, y + 1, i);
                        bool canGoLeft      = canSwap(x - 1, y, i);
                        bool canGoRight     = canSwap(x + 1, y, i);

                        // Always prioritise y down
                        if (canSwap(x, y + 1, i)) {
                            move(i, x, y + 1);
                        } else if (canGoDownLeft && canGoDownRight) {
                            if (coinFlip()) {
                                move(i, x - 1, y + 1);
                            } else {
                                move(i, x + 1, y + 1);
                            }
                        } else if (canGoDownLeft) {
                            move(i, x - 1, y + 1);
                        } else if (canGoDownRight) {
                            move(i, x + 1, y + 1);
                        } else if (canGoLeft && canGoRight) {
                            if (coinFlip()) {
                                move(i, x - 1, y);
                            } else {
                                move(i, x + 1, y);
                            }
                        } else if (canGoLeft) {
                            move(i, x - 1, y);
                        } else if (canGoRight) {
                            move(i, x + 1, y);
                        }

                        break;
                    }
                    case mat::Solid: {
                        if (!materialRegistry.getItem(particles[i].materialId).hasStructure) {
                            int deltaY = 0;
                            if (y + 1 < static_cast<int64_t>(height))
                                deltaY = +1;

                            if (canSwap(x, y + deltaY, i)) {
                                move(i, x, y + deltaY);
                                break;
                            }

                            deltaY = 0;
                            if (y + 2 < static_cast<int64_t>(height))
                                deltaY = +2;
                            if (deltaY == 0)
                                break;

                            uint8_t possibleSides = 0b00;
                            if (!(x + -1 < 0) && canSwap(x + -1, y + deltaY, i))
                                possibleSides |= 0b10;
                            if (x + 1 < static_cast<int64_t>(this->width) && canSwap(x + 1, y + deltaY, i))
                                possibleSides |= 0b01;

                            if (possibleSides == 0b11) {
                                if (coinFlip()) {
                                    move(i, x - 1, y + deltaY);
                                } else {
                                    move(i, x + 1, y + deltaY);
                                }
                            } else if (possibleSides & 0b10) {
                                move(i, x - 1, y + deltaY);
                            } else if (possibleSides & 0b01) {
                                move(i, x + 1, y + deltaY);
                            }
                        }

                        break;
                    }
                    case mat::Gas: {
                        bool leftRight = coinFlip();
                        bool topBottom = coinFlip();

                        int deltaX = leftRight ? 1 : -1;
                        int deltaY = topBottom ? 1 : -1;

                        if (canSwap(x + deltaX, y + deltaY, i)) {
                            move(i, x + deltaX, y + deltaY);
                        }

                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }

    void Simulation::updatePixelBuffer() {
        std::lock_guard<std::mutex> lock(particleAccessMutex);

        for (size_t i = 0; i < this->particles.size(); i++) {
            if (!this->particles[i].occupied) {
                std::memset(pixelBuffer.data() + (i * 4), 0, 4);
                continue;
            }

            uint32_t colour = 0x000000FF;

            switch (this->renderingMode) {
            case ShowColour:
                colour = materialRegistry.getItem(this->particles[i].materialId).colour;
                break;
            case ShowPressure: {

                if (particles[i].state != mat::Liquid) {
                    colour = materialRegistry.getItem(this->particles[i].materialId).colour;
                } else {
                    colour = utils::linearlyInterpolateColour(minCompression, maxCompression, 0xFF0000FF,
                                                              0xFFFFFFFF, particles[i].compression);
                }

                break;
            }
            }

            this->pixelBuffer[(i * 4) + 0] = static_cast<uint8_t>((colour >> 24) & 0xFF);
            this->pixelBuffer[(i * 4) + 1] = static_cast<uint8_t>((colour >> 16) & 0xFF);
            this->pixelBuffer[(i * 4) + 2] = static_cast<uint8_t>((colour >> 8) & 0xFF);
            this->pixelBuffer[(i * 4) + 3] = static_cast<uint8_t>(colour & 0xFF);
        }
    }

    const mat::Particle &Simulation::operator[](size_t idx) const {
        return particles.at(static_cast<size_t>(idx));
    }

    mat::Particle &Simulation::operator[](size_t idx) {
        return const_cast<mat::Particle &>(std::as_const(*this)[idx]);
    }

    bool Simulation::coinFlip() {
        if (bitsLeft == 0) {
            randomBitBuffer = rng();
            bitsLeft        = 64;
        }

        bool result = randomBitBuffer & 1;
        --bitsLeft;
        randomBitBuffer >>= 1;

        return result;
    }

    // Swapping logic for fluids (gas, liquid), and non-structural solids, e.g.
    // sand
    bool Simulation::canSwap(int x, int y, int idx) {

        if (x < 0 || static_cast<size_t>(x) >= width || y < 0 || static_cast<size_t>(y) >= height) {
            return false;
        }

        auto ref = &particles[utils::xyToIdx(x, y, this->width)];

        if (!ref->occupied) {
            return true;
        } else if (ref->occupied && ref->state == mat::Liquid && particles[idx].state == mat::Solid &&
                   !materialRegistry.getItem(particles[idx].materialId).hasStructure &&
                   materialRegistry.getItem(ref->materialId).mass <
                       materialRegistry.getItem(particles[idx].materialId)
                           .mass) // non-structural solids sinking in fluids
        {
            return true;
        } else if (ref->occupied && (ref->state == mat::Liquid || ref->state == mat::Gas) &&
                   (particles[idx].state == mat::Liquid || particles[idx].state == mat::Gas) &&
                   materialRegistry.getItem(ref->materialId).mass <
                       materialRegistry.getItem(particles[idx].materialId).mass // fluids sinking in fluids
        ) {
            return true;
        }

        return false;
    }

    const uint8_t *Simulation::getPixelBuffer() { return this->pixelBuffer.data(); }

    int Simulation::drawRectangle(int x, int y, int width, int height, const int16_t materialId, bool fill,
                                  bool replace) {

        int particlesDrawn = 0;

        x      = std::clamp<int>(x, 0, this->width - 1);
        y      = std::clamp<int>(y, 0, this->height - 1);
        width  = std::clamp<int>(x + width - 1, 0, this->width - 1);
        height = std::clamp<int>(y + height - 1, 0, this->height - 1);

        int drawX0 = width < x ? width : x;
        int drawX1 = width < x ? x : width;
        int drawY0 = height < y ? height : y;
        int drawY1 = height < y ? y : height;

        for (int i = drawY0; i <= drawY1; i++) {
            for (int j = drawX0; j <= drawX1; j++) {
                int  idx        = utils::xyToIdx(j, i, this->width);
                bool canReplace = replace || (!replace && !this->particles[idx].occupied);

                if (canReplace && (fill || (i == drawY0 || i == drawY1 || j == drawY0 || j == drawY1))) {
                    this->particles[idx] = {
                        .materialId = materialId,
                        .state      = mat::decideState(this->materialRegistry.getItem(materialId),
                                                       this->ambientTemperature),
                        .occupied   = true,
                    };
                    particlesDrawn++;
                }
            }
        }

        return particlesDrawn;
    }

    void Simulation::printDebugInfo() {

        logging::message(std::format("Simulation size: {} x {}", this->width, this->height));
        logging::message(
            std::format("Memory used by particles: {}", this->particles.size() * sizeof(mat::Particle)));
        logging::message(
            std::format("Memory used by pixels: {}", this->pixelBuffer.size() * sizeof(uint8_t)));
    }
} // namespace sim