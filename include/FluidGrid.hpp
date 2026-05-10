#pragma once
#include <vector>
#include "raylib.h"

struct FluidConfig {
    float cellSize = 1.0f;
    float density = 1.0f;
    float deltaTime = 1.0f / 60.0f;
    int pressureIterations = 10;
};

class FluidGrid {
public:
    const int cellCountX;
    const int cellCountY;
    FluidConfig config;

    std::vector<float> velX; // (cellCountX + 1) * cellCountY
    std::vector<float> velY; // cellCountX * (cellCountY + 1)
    std::vector<float> velX_temp;
    std::vector<float> velY_temp;
    std::vector<float> pressure; // cellCountX * cellCountY
    std::vector<bool> solids; // cellCountX * cellCountY

    FluidGrid(int cellCountX, int cellCountY, FluidConfig config);

    // Coordinate helpers
    inline int idx(int x, int y) const { return y * cellCountX + x; }
    inline int idxX(int x, int y) const { return y * (cellCountX + 1) + x; }
    inline int idxY(int x, int y) const { return y * cellCountX + x; }

    Vector2 getBottomLeft() const;
    Vector2 cellCenter(int x, int y) const;
    Vector2 cellBottomLeft(int x, int y) const;
    Vector2 leftEdgeCenter(int x, int y) const;
    Vector2 bottomEdgeCenter(int x, int y) const;

    bool isSolid(int x, int y) const;
    float getPressure(int x, int y) const;

    void randomizeVelXY();
    void reset();
    int calculateDivergenceError();

    void solvePressure(float weightSOR = 1);
    float solvePressureAtCell(int x, int y);
    void updateVelocities();
    void advectVelocities();
    float calculateDivVelocityAtCell(int x, int y) const;
    void update();

    static float bilinearSample(const std::vector<float>& edgeValues, Vector2 edgeValueDimensions, float cellSize, Vector2 worldPos);
    Vector2 getVelocityAtWorldPos(Vector2 worldPos);
};
