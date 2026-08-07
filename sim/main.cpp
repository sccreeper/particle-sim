#include <raylib.h>
#include <iostream>
#include <cstdint>
#include "registry.hpp"

int main() {

    InitWindow(640, 480, "Particle Sim");

    std::cout << "HELLPPP" << std::endl;

    int64_t counter = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLUE);
        DrawRectangle(10, 20, 30, 40, RED);

        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}