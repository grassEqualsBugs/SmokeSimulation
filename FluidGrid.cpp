#include "include/FluidGrid.hpp"
#include <random>

FluidGrid::FluidGrid(int cellCountX, int cellCountY, int cellSize)
    : cellCountX(cellCountX), cellCountY(cellCountY), cellSize(cellSize),
      velX(cellCountX + 1, std::vector<float>(cellCountY, 0.f)),
      velY(cellCountX, std::vector<float>(cellCountY + 1, 0.f)) {
    randomizeVelXY();
}

void FluidGrid::randomizeVelXY() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.f, 1.f);

    for (auto& row : velX)
        for (auto& v : row)
            v = dist(rng);

    for (auto& row : velY)
        for (auto& v : row)
            v = dist(rng);
}

float FluidGrid::calculateDivVelocityAtCell(int cellX, int cellY) {
    float top = velY[cellX][cellY+1];
    float bottom = velY[cellX][cellY];
    float left = velX[cellX][cellY];
    float right = velX[cellX+1][cellY];
    float gradX = (right - left) / cellSize;
    float gradY = (top - bottom) / cellSize;
    float div = gradX + gradY;
    return div;
}
