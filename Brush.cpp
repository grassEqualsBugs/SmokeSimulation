#include "include/Brush.hpp"
#include "include/raymath.h"

Brush::Brush(FluidGrid& grid, float radius)
    : fluidGrid(grid), radius(radius), lastMouseWorldPos({0, 0}), isMousePressed(false) {}

void Brush::update(Camera2D camera) {
    Vector2 mousePos = GetMousePosition();
    Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);
    // Since world is flipped in render, we flip it back for logic if needed
    // But GetScreenToWorld2D handles the camera's zoom/offset.
    // However, in main.cpp, the vis is rendered inside an rlScalef(1, -1, 1).
    // To match world coordinates, we flip the Y.
    mouseWorldPos.y *= -1.0f;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isMousePressed = true;
        lastMouseWorldPos = mouseWorldPos;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isMousePressed = false;
    }

    if (isMousePressed) {
        Vector2 mouseDelta = Vector2Subtract(mouseWorldPos, lastMouseWorldPos);

        float h = fluidGrid.config.cellSize;
        float halfH = h * 0.5f;

        // Calculate grid bounds for optimization (approximate)
        float gridW = (float)fluidGrid.cellCountX * h;
        float gridH = (float)fluidGrid.cellCountY * h;
        Vector2 gridOrigin = { -gridW * 0.5f, -gridH * 0.5f };

        // Update velX
        for (int x = 0; x <= fluidGrid.cellCountX; x++) {
            for (int y = 0; y < fluidGrid.cellCountY; y++) {
                // X-velocity is on vertical edges
                Vector2 pos = { gridOrigin.x + (float)x * h, gridOrigin.y + ((float)y + 0.5f) * h };
                if (CheckCollisionPointCircle(pos, mouseWorldPos, radius)) {
                    fluidGrid.velX[fluidGrid.idxX(x, y)] += mouseDelta.x;
                }
            }
        }

        // Update velY
        for (int x = 0; x < fluidGrid.cellCountX; x++) {
            for (int y = 0; y <= fluidGrid.cellCountY; y++) {
                // Y-velocity is on horizontal edges
                Vector2 pos = { gridOrigin.x + ((float)x + 0.5f) * h, gridOrigin.y + (float)y * h };
                if (CheckCollisionPointCircle(pos, mouseWorldPos, radius)) {
                    fluidGrid.velY[fluidGrid.idxY(x, y)] += mouseDelta.y;
                }
            }
        }

        lastMouseWorldPos = mouseWorldPos;
    }
}

void Brush::render(Camera2D camera) {
    Vector2 mousePos = GetMousePosition();
    Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);

    Color col = isMousePressed ? (Color){0, 255, 0, 100} : (Color){255, 255, 255, 100};

    mouseWorldPos.y *= -1.0f;
    DrawCircleV(mouseWorldPos, radius, col);
}
