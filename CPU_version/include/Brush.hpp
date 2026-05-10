#pragma once
#include "raylib.h"
#include "FluidGrid.hpp"

class Brush {
public:
    FluidGrid& fluidGrid;
    float radius;
    float velocityStrength;

    Brush(FluidGrid& grid, float radius, float velocityStrength = 1.0f);

    void update(Camera2D camera);
    void render(Camera2D camera);

private:
    void updateVelocity(Vector2 mouseWorldPos, Vector2 mouseDelta);
    void updateSmoke(Vector2 mouseWorldPos);

    Vector2 lastMouseWorldPos;
    bool isMousePressed;
};
