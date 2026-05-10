#pragma once
#include "raylib.h"
#include "FluidGrid.hpp"

class Brush {
public:
    FluidGrid& fluidGrid;
    float radius;

    Brush(FluidGrid& grid, float radius);

    void update(Camera2D camera);
    void render(Camera2D camera);

private:
    Vector2 lastMouseWorldPos;
    bool isMousePressed;
};
