#pragma once
#include "raylib/raylib.h"
#include "FluidGrid.hpp"
#include <functional>

enum class GridVisMode {
    DIVERGENCE,
    SPEED,
    SMOKE
};

class GridVisualization {
public:
    FluidGrid& fluidGrid;
    Vector2 cellDisplaySize;
    Vector2 boundsSize;
    float cellBorderThickness = 0.03f;
    float speedVisMax = 1.5f;
    float divergenceColorRange = 0.4f;
    float interpolatedVelocityArrowThickness = 0.018f;
    float velocityArrowThickness = 0.02f;
    float halfCellSize;
    int interpolatedVelocitiesPerSide;
    GridVisMode visMode;

    GridVisualization(FluidGrid& fluidGrid, int interpolatedVelocitiesPerSide, GridVisMode visMode);

    void renderGrid();
    void drawDebugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback);

private:
    void drawCells();
    void drawVelX();
    void drawVelY();
    void drawInterpolatedVelocities();
};
