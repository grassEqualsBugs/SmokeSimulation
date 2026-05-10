#pragma once
#include "raylib.h"
#include "FluidGrid.hpp"
#include <string>
#include <functional>

class GridVisualization {
public:
    FluidGrid& fluidGrid;
    Vector2 cellDisplaySize;
    Vector2 boundsSize;
    float cellBorderThickness = 0.03f;
    float divergenceColorRange = 0.3f;
    float interpolatedVelocityArrowThickness = 0.012f;
    float velocityArrowThickness = 0.02f;
    float halfCellSize;
    int interpolatedVelocitiesPerSide;

    GridVisualization(FluidGrid& fluidGrid, int interpolatedVelocitiesPerSide);

    void renderGrid();
    void drawDebugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback);

private:
    void drawCells();
    void drawVelX();
    void drawVelY();
    void drawInterpolatedVelocities();
};
