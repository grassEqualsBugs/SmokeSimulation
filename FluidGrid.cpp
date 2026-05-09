#include "include/FluidGrid.hpp"
#include "include/raymath.h"
#include <random>
#include <functional>
#include <algorithm>

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
    // randomizeVelXY();
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

Vector2 FluidGrid::getVelocityAtWorldPos(Vector2 worldPos) {
    float velX = FluidGrid::bilinearSample(FluidGrid::velX, (Vector2){cellCountX + 1.f, cellCountY + 0.f}, config.cellSize, worldPos);
    float velY = FluidGrid::bilinearSample(FluidGrid::velY, (Vector2){cellCountX + 0.f, cellCountY + 1.f}, config.cellSize, worldPos);
    return (Vector2){velX, velY};
}

float FluidGrid::bilinearSample(const std::vector<float>& edgeValues, Vector2 edgeValueDimensions, float cellSize, Vector2 worldPos) {
    int edgeCountX = edgeValueDimensions.x;
    int edgeCountY = edgeValueDimensions.y;
    std::function<int(int,int)> edgeIdx = [edgeCountX](int x, int y) { return edgeCountX * y + x; };

    float width = (edgeCountX - 1) * cellSize;
    float height = (edgeCountY - 1) * cellSize;

    // indices of cell we are sampling in
    float px = (worldPos.x + width / 2) / cellSize; // range is between [0, countX]
    float py = (worldPos.y + height / 2) / cellSize; // range is between [0, count Y]

    int left = std::clamp((int)px, 0, edgeCountX - 2);
    int bottom = std::clamp((int)py, 0, edgeCountY - 2);
    int right = left + 1;
    int top = bottom + 1;

    float xt = std::clamp(px - left, 0.f, 1.f);
    float yt = std::clamp(py - bottom, 0.f, 1.f);
    float valTop = Lerp(edgeValues[edgeIdx(left, top)], edgeValues[edgeIdx(right, top)], xt);
    float valBottom = Lerp(edgeValues[edgeIdx(left, bottom)], edgeValues[edgeIdx(right, bottom)], xt);
    return Lerp(valBottom, valTop, yt);
}
