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
    Vector2 bottomLeft;
    float cellBorderThickness = 0.03f;
    float divergenceColorRange = 3.f;
    float interpolatedVelocityArrowThickness = 0.012f;
    float velocityArrowThickness = 0.02f;
    float halfCellSize;
    int interpolatedVelocitiesPerSide;

    GridVisualization(FluidGrid& fluidGrid, int interpolatedVelocitiesPerSide);

    Vector2 cellCenter(int x, int y) const;
    Vector2 cellBottomLeft(int x, int y) const;
    Vector2 leftEdgeCenter(int x, int y) const;
    Vector2 bottomEdgeCenter(int x, int y) const;

    void renderGrid();
    void drawDebugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback);

private:
    void drawCells();
    void drawVelX();
    void drawVelY();
    void drawInterpolatedVelocities();
};
