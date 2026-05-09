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
    float velocityRectangleThickness = 0.2f;
    float halfCellSize;

    GridVisualization(FluidGrid& fluidGrid);

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
};
