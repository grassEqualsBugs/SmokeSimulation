#pragma once
#include <vector>

class FluidGrid {
public:
    int cellCountX;
    int cellCountY;
    float cellSize;
    std::vector<std::vector<float>> velX;
    std::vector<std::vector<float>> velY;

    std::vector<std::vector<bool>> solidCellMap;
    bool isSolid(int x, int y);

    float deltaTime = 1 / 60.f;
    float density = 1;

    std::vector<std::vector<float>> pressureMap;
    float getPressure(int x, int y);
    float pressureSolveCell(int x, int y);
    void solvePressure();
    void updateVelocities();

    FluidGrid(int cellCountX, int cellCountY, float cellSize);
    void randomizeVelXY();
    float calculateDivVelocityAtCell(int x, int y);
};
