#include "include/FluidGrid.hpp"
#include <random>

FluidGrid::FluidGrid(int cellCountX, int cellCountY, FluidConfig config)
    : cellCountX(cellCountX), cellCountY(cellCountY), config(config),
      velX((cellCountX + 1) * cellCountY, 0.f),
      velY(cellCountX * (cellCountY + 1), 0.f),
      pressure(cellCountX * cellCountY, 0.f),
      solids(cellCountX * cellCountY, false) {

    for (int x = 0; x < cellCountX; x++) {
        solids[idx(x, 0)] = true;
        solids[idx(x, cellCountY - 1)] = true;
    }
    for (int y = 0; y < cellCountY; y++) {
        solids[idx(0, y)] = true;
        solids[idx(cellCountX - 1, y)] = true;
    }
    randomizeVelXY();
}

bool FluidGrid::isSolid(int x, int y) const {
    if (x < 0 || x >= cellCountX || y < 0 || y >= cellCountY) return true;
    return solids[idx(x, y)];
}

float FluidGrid::getPressure(int x, int y) const {
    if (x < 0 || x >= cellCountX || y < 0 || y >= cellCountY) return 0.0f;
    return pressure[idx(x, y)];
}

void FluidGrid::randomizeVelXY() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.f, 1.f);
    for (float& v : velX) v = dist(rng);
    for (float& v : velY) v = dist(rng);
}

float FluidGrid::solvePressureAtCell(int x, int y) {
    if (isSolid(x, y)) return 0.f;

    int flowT = !isSolid(x, y + 1);
    int flowB = !isSolid(x, y - 1);
    int flowL = !isSolid(x - 1, y);
    int flowR = !isSolid(x + 1, y);
    int n = flowT + flowB + flowL + flowR;
    if (n == 0) return 0.f;

    float delVelocitySum = (velX[idxX(x + 1, y)] - velX[idxX(x, y)] + velY[idxY(x, y + 1)] - velY[idxY(x, y)]);
    float pSum = (flowR ? getPressure(x + 1, y) : 0) +
                 (flowL ? getPressure(x - 1, y) : 0) +
                 (flowT ? getPressure(x, y + 1) : 0) +
                 (flowB ? getPressure(x, y - 1) : 0);

    return (pSum - config.density * config.cellSize * delVelocitySum / config.deltaTime) / n;
}

void FluidGrid::solvePressure() {
    for (int iter = 0; iter < config.pressureIterations; iter++) {
        for (int x = 0; x < cellCountX; x++) {
            for (int y = 0; y < cellCountY; y++) {
                pressure[idx(x, y)] = solvePressureAtCell(x, y);
            }
        }
    }
}

void FluidGrid::updateVelocities() {
    const float K = config.deltaTime / (config.density * config.cellSize);

    for (int x = 0; x <= cellCountX; x++) {
        for (int y = 0; y < cellCountY; y++) {
            if (isSolid(x, y) || isSolid(x - 1, y)) {
                velX[idxX(x, y)] = 0;
            } else {
                velX[idxX(x, y)] -= K * (getPressure(x, y) - getPressure(x - 1, y));
            }
        }
    }

    for (int x = 0; x < cellCountX; x++) {
        for (int y = 0; y <= cellCountY; y++) {
            if (isSolid(x, y) || isSolid(x, y - 1)) {
                velY[idxY(x, y)] = 0;
            } else {
                velY[idxY(x, y)] -= K * (getPressure(x, y) - getPressure(x, y - 1));
            }
        }
    }
}

float FluidGrid::calculateDivVelocityAtCell(int x, int y) const {
    if (isSolid(x, y)) return 0.0f;
    float div = (velX[idxX(x + 1, y)] - velX[idxX(x, y)] +
                 velY[idxY(x, y + 1)] - velY[idxY(x, y)]) / config.cellSize;
    return div;
}

float FluidGrid::bilinearSample(const std::vector<float>& field, int resX, int resY, float cellSize, Vector2 worldPos) {
    return 0.0f;
}
