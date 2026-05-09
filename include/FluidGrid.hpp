#pragma once
#include "raylib.h"
#include <vector>

class FluidGrid {
public:
    int cellCountX;
    int cellCountY;
    int cellSize;
    std::vector<std::vector<float>> velX;
    std::vector<std::vector<float>> velY;

    FluidGrid(int cellCountX, int cellCountY, int cellSize);
    void randomizeVelXY();
    float calculateDivVelocityAtCell(int cellX, int cellY);
};
