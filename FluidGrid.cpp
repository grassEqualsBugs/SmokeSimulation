#include "include/FluidGrid.hpp"
#include <random>

FluidGrid::FluidGrid(int cellCountX, int cellCountY, float cellSize)
    : cellCountX(cellCountX), cellCountY(cellCountY), cellSize(cellSize),
      velX(cellCountX + 1, std::vector<float>(cellCountY, 0.f)),
      velY(cellCountX, std::vector<float>(cellCountY + 1, 0.f)),
      pressureMap(cellCountX, std::vector<float>(cellCountY, 0.f)) {
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

float FluidGrid::getPressure(int x, int y) {
    bool outOfBounds = x < 0 || x >= cellCountX || y < 0 || y >= cellCountY;
    return outOfBounds ? 0 : pressureMap[x][y];
}

void FluidGrid::pressureSolveCell(int x, int y) {
    float pressureTop = getPressure(x, y+1);
    float pressureLeft = getPressure(x-1, y);
    float pressureRight = getPressure(x+1, y);
    float pressureBottom = getPressure(x, y-1);
    float velocityTop = velY[x][y+1];
    float velocityLeft = velX[x][y];
    float velocityRight = velX[x+1][y];
    float velocityBottom = velY[x][y];
    float pressureSum = pressureRight + pressureLeft + pressureTop + pressureBottom;
    float deltaVelocitySum = velocityRight - velocityLeft + velocityTop - velocityBottom;
    pressureMap[x][y] = (pressureSum - density * cellSize * deltaVelocitySum / deltaTime) / 4.f;
}

void FluidGrid::solvePressure() {
    for (int x = 0; x < cellCountX; x++) {
        for (int y = 0; y < cellCountY; y++) {
            pressureSolveCell(x, y);
        }
    }
}

void FluidGrid::updateVelocities() {
    const float K = deltaTime / (density * cellSize);
    for (int x = 0; x < velX.size(); x++) {
        for (int y = 0; y < velX[0].size(); y++) {
            float pressureRight = getPressure(x, y);
            float pressureLeft = getPressure(x - 1, y);
            velX[x][y] -= K * (pressureRight - pressureLeft);
        }
    }

    for (int x = 0; x < velY.size(); x++) {
        for (int y = 0; y < velY[0].size(); y++) {
            float pressureTop = getPressure(x, y);
            float pressureBottom = getPressure(x, y - 1);
            velX[x][y] -= K * (pressureTop - pressureBottom);
        }
    }
}

float FluidGrid::calculateDivVelocityAtCell(int x, int y) {
    float top = velY[x][y+1];
    float bottom = velY[x][y];
    float left = velX[x][y];
    float right = velX[x+1][y];
    float gradX = (right - left) / cellSize;
    float gradY = (top - bottom) / cellSize;
    float div = gradX + gradY;
    return div;
}
