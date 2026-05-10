#include "include/Brush.hpp"
#include "include/raymath.h"

Brush::Brush(FluidGrid& grid, float radius, float velocityStrength)
    : fluidGrid(grid), radius(radius), velocityStrength(velocityStrength), lastMouseWorldPos({0, 0}), isMousePressed(false) {}

void Brush::update(Camera2D camera) {
    float scroll = GetMouseWheelMove();
    if (scroll != 0) {
        radius += scroll * 0.05f;
        if (radius < 0.05f) radius = 0.05f;
    }

    Vector2 mousePos = GetMousePosition();
    Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);
    mouseWorldPos.y *= -1.0f;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        isMousePressed = true;
        Vector2 mouseDelta = Vector2Subtract(mouseWorldPos, lastMouseWorldPos);

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            updateVelocity(mouseWorldPos, mouseDelta);
        }

        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            updateSmoke(mouseWorldPos);
        }

        lastMouseWorldPos = mouseWorldPos;
    } else {
        isMousePressed = false;
        lastMouseWorldPos = mouseWorldPos;
    }
}

void Brush::updateVelocity(Vector2 mouseWorldPos, Vector2 mouseDelta) {
    float h = fluidGrid.config.cellSize;
    float gridW = (float)fluidGrid.cellCountX * h;
    float gridH = (float)fluidGrid.cellCountY * h;
    Vector2 gridOrigin = { -gridW * 0.5f, -gridH * 0.5f };

    for (int x = 0; x <= fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 pos = { gridOrigin.x + (float)x * h, gridOrigin.y + ((float)y + 0.5f) * h };
            if (CheckCollisionPointCircle(pos, mouseWorldPos, radius)) {
                fluidGrid.velX[fluidGrid.idxX(x, y)] += mouseDelta.x * velocityStrength;
            }
        }
    }

    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y <= fluidGrid.cellCountY; y++) {
            Vector2 pos = { gridOrigin.x + ((float)x + 0.5f) * h, gridOrigin.y + (float)y * h };
            if (CheckCollisionPointCircle(pos, mouseWorldPos, radius)) {
                fluidGrid.velY[fluidGrid.idxY(x, y)] += mouseDelta.y * velocityStrength;
            }
        }
    }
}

void Brush::updateSmoke(Vector2 mouseWorldPos) {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 pos = fluidGrid.cellCenter(x, y);
            if (CheckCollisionPointCircle(pos, mouseWorldPos, radius)) {
                fluidGrid.smokeMap[fluidGrid.idx(x, y)] = 1.0f;
            }
        }
    }
}

void Brush::render(Camera2D camera) {
    Vector2 mousePos = GetMousePosition();
    Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);

    Color col = isMousePressed ? (Color){0, 255, 0, 100} : (Color){255, 255, 255, 100};

    mouseWorldPos.y *= -1.0f;
    DrawCircleLinesV(mouseWorldPos, radius, col);
    DrawCircleV(mouseWorldPos, radius, (Color){col.r, col.g, col.b, 40});
}
