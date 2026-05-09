#include "include/FluidGrid.hpp"
#include <random>

FluidGrid::FluidGrid(int cellCountX, int cellCountY, float cellSize)
    : cellCountX(cellCountX), cellCountY(cellCountY), cellSize(cellSize),
      velX(cellCountX + 1, std::vector<float>(cellCountY, 0.f)),
      velY(cellCountX, std::vector<float>(cellCountY + 1, 0.f)),
      solidCellMap(cellCountX, std::vector<bool>(cellCountY, false)),
      pressureMap(cellCountX, std::vector<float>(cellCountY, 0.f)) {
    for (int x = 0; x < cellCountX; x++) {
        solidCellMap[x][0] = true;
        solidCellMap[x][cellCountY - 1] = true;
    }
    for (int y = 0; y < cellCountY; y++) {
        solidCellMap[0][y] = true;
        solidCellMap[cellCountX - 1][y] = true;
    }
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

bool FluidGrid::isSolid(int x, int y) {
    bool outOfBounds = x < 0 || x >= cellCountX || y < 0 || y >= cellCountY;
    return outOfBounds || solidCellMap[x][y];
}

float FluidGrid::pressureSolveCell(int x, int y) {
    int flowTop = !isSolid(x, y + 1);
    int flowLeft = !isSolid(x - 1, y);
    int flowRight = !isSolid(x + 1, y);
    int flowBottom = !isSolid(x, y - 1);
    int fluidEdgeCount = flowLeft + flowRight + flowTop + flowBottom;
    if (isSolid(x, y) || fluidEdgeCount == 0) return 0.f;

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
    return (pressureSum - density * cellSize * deltaVelocitySum / deltaTime) / fluidEdgeCount;
}

void FluidGrid::solvePressure() {
    for (int iteration = 0; iteration < 40; iteration++) {
        for (int x = 0; x < cellCountX; x++) {
            for (int y = 0; y < cellCountY; y++) {
                pressureMap[x][y] = pressureSolveCell(x, y);
            }
        }
    }
}

void FluidGrid::updateVelocities() {
    const float K = deltaTime / (density * cellSize);
    for (int x = 0; x < velX.size(); x++) {
        for (int y = 0; y < velX[0].size(); y++) {
            if (isSolid(x, y) || isSolid(x - 1, y)) {
                velX[x][y] = 0;
                continue;
            }
            float pressureRight = getPressure(x, y);
            float pressureLeft = getPressure(x - 1, y);
            velX[x][y] -= K * (pressureRight - pressureLeft);
        }
    }

    for (int x = 0; x < velY.size(); x++) {
        for (int y = 0; y < velY[0].size(); y++) {
            if (isSolid(x, y) || isSolid(x, y - 1)) {
                velY[x][y] = 0;
                continue;
            }
            float pressureTop = getPressure(x, y);
            float pressureBottom = getPressure(x, y - 1);
            velY[x][y] -= K * (pressureTop - pressureBottom);
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
