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

    void DrawOutlinedText(const char *text, int posX, int posY, int fontSize, Color color, int outlineSize, Color outlineColor);

    Vector2 cellCenter(int x, int y);
    Vector2 cellBottomLeft(int x, int y);
    Vector2 leftEdgeCenter(int x, int y);
    Vector2 bottomEdgeCenter(int x, int y);

    void renderGrid();
    void debugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback);
};
