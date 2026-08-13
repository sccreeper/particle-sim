#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>

#include <raylib.h>
#include <rlgl.h>
#include <stdlib.h>

#include "logging.hpp"
#include "materials.hpp"
#include "registry.hpp"
#include "simulation.hpp"

const int SIM_WIDTH = 512;
const int SIM_HEIGHT = 512;
const int DEFAULT_FONT_SIZE = 16;

int main() {

    InitWindow(SIM_WIDTH, SIM_HEIGHT, "Particle Sim");

    auto simulation = sim::Simulation{SIM_WIDTH, SIM_HEIGHT};
    simulation.printDebugInfo();
    auto renderTexture = LoadRenderTexture(SIM_WIDTH, SIM_HEIGHT);

    uint16_t selectedMaterial = 0;

    int toolOriginX = 0;
    int toolOriginY = 0;
    bool toolBeingUsed = false;

    bool paused = true;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Handle keypresses
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        } else if (paused && IsKeyPressed(KEY_Q)) {
            simulation.tick();
        }

        bool rPressed = IsKeyPressed(KEY_R);
        bool tPressed = IsKeyPressed(KEY_T);
        if (rPressed || tPressed) {

            bool goRight = rPressed;

            if (goRight && selectedMaterial == simulation.materialRegistry.getLast()) {
                selectedMaterial = simulation.materialRegistry.getFirst();
            } else if (!goRight && selectedMaterial == simulation.materialRegistry.getFirst()) {
                selectedMaterial = simulation.materialRegistry.getLast();
            } else {
                int newId = static_cast<int>(selectedMaterial) + (goRight ? 1 : -1);
                while (!std::ranges::contains(simulation.materialRegistry.ids(), newId)) {
                    if (static_cast<int>(selectedMaterial) - 1 < simulation.materialRegistry.getFirst()) {
                        newId = simulation.materialRegistry.getLast();
                        continue;
                    }

                    newId = newId + (goRight ? 1 : -1);
                }

                selectedMaterial = static_cast<uint16_t>(newId);
            }
        }

        if (!paused) {
            simulation.tick();
        }
        simulation.updatePixelBuffer();

        // Update render texture
        rlUpdateTexture(renderTexture.texture.id, 0, 0, SIM_WIDTH, SIM_WIDTH,
                        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, simulation.getPixelBuffer());

        // Drawing
        BeginDrawing();

        ClearBackground(BLACK);

        DrawTexture(renderTexture.texture, 0, 0, WHITE);
        DrawText(std::format("Material: {} \n{}", simulation.materialRegistry.getItem(selectedMaterial).name,
                             paused ? "Paused" : "Running")
                     .c_str(),
                 10, 10, DEFAULT_FONT_SIZE, WHITE);

        if (toolBeingUsed) {

            DrawRectangleLines((GetMouseX() > toolOriginX ? toolOriginX : GetMouseX()),
                               (GetMouseY() > toolOriginY ? toolOriginY : GetMouseY()),
                               std::abs(GetMouseX() - toolOriginX), std::abs(GetMouseY() - toolOriginY), RED);

            DrawRectangle(toolOriginX - 2, toolOriginY - 2, 4, 4, WHITE);
            DrawRectangle(GetMouseX() - 2, GetMouseY() - 2, 4, 4, WHITE);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (!toolBeingUsed) {
                toolOriginX = GetMouseX();
                toolOriginY = GetMouseY();
                toolBeingUsed = true;
            } else {
                int numDrawn =
                    simulation.drawRectangle((GetMouseX() > toolOriginX ? toolOriginX : GetMouseX()),
                                             (GetMouseY() > toolOriginY ? toolOriginY : GetMouseY()),
                                             std::abs(GetMouseX() - toolOriginX),
                                             std::abs(GetMouseY() - toolOriginY), selectedMaterial);

                toolBeingUsed = false;

                logging::message(std::format("{} {} particles drawn", numDrawn,
                                             simulation.materialRegistry.getItem(selectedMaterial).name));
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}