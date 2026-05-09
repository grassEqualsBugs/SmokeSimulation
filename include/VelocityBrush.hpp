#pragma once
#include "raylib.h"
#include "FluidGrid.hpp"

class VelocityBrush {
public:
    FluidGrid& fluidGrid;
    float radius;

    VelocityBrush(FluidGrid& grid, float radius);

    void update(Camera2D camera);
    void render(Camera2D camera);

private:
    Vector2 lastMouseWorldPos;
    bool isMousePressed;
};
