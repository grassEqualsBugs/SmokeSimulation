#include "include/GridVisualization.hpp"
#include "include/RaylibUtils.hpp"
#include "include/rlgl.h"
#include "include/raymath.h"
#include <cmath>

using namespace RaylibUtils;

GridVisualization::GridVisualization(FluidGrid& fluidGrid)
    : fluidGrid(fluidGrid) {
    const float h = fluidGrid.config.cellSize;
    cellDisplaySize = Vector2Scale(Vector2One(), h * (1.0f - cellBorderThickness));
    boundsSize = (Vector2){ (float)fluidGrid.cellCountX * h, (float)fluidGrid.cellCountY * h };
    bottomLeft = Vector2Scale(boundsSize, -0.5f);
    halfCellSize = h * 0.5f;
}

Vector2 GridVisualization::cellCenter(int x, int y) const {
    const float h = fluidGrid.config.cellSize;
    return Vector2Add(bottomLeft, Vector2Scale((Vector2){x + 0.5f, y + 0.5f}, h));
}

Vector2 GridVisualization::cellBottomLeft(int x, int y) const {
    const float h = fluidGrid.config.cellSize;
    return Vector2Add(bottomLeft, Vector2Scale((Vector2){(float)x, (float)y}, h));
}

Vector2 GridVisualization::leftEdgeCenter(int x, int y) const {
    return Vector2Subtract(cellCenter(x, y), (Vector2){halfCellSize, 0.0f});
}

Vector2 GridVisualization::bottomEdgeCenter(int x, int y) const {
    return Vector2Subtract(cellCenter(x, y), (Vector2){0.0f, halfCellSize});
}

void GridVisualization::renderGrid() {
    drawCells();
    drawVelX();
    drawVelY();
}

void GridVisualization::drawCells() {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 center = cellCenter(x, y);
            Vector2 offset = Vector2Scale(cellDisplaySize, 0.5f);
            Vector2 pos = Vector2Subtract(center, offset);

            Color col;
            if (fluidGrid.isSolid(x, y)) {
                col = (Color){40, 40, 40, 255};
            } else {
                float div = fluidGrid.calculateDivVelocityAtCell(x, y);
                float t = fminf(fabsf(div) / 5.0f, 1.0f);
                Color target = (div < 0) ? Color{245, 66, 66, 255} : Color{66, 135, 245, 255};
                col = Vec4ToColor(Vector4Lerp(ColorToVec4(DARKGRAY), ColorToVec4(target), t));
            }
            DrawRectangleV(pos, cellDisplaySize, col);
        }
    }
}

void GridVisualization::drawVelX() {
    for (int x = 0; x <= fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            float val = fluidGrid.velX[fluidGrid.idxX(x, y)] * halfCellSize;
            float width = fabsf(val);
            float height = halfCellSize * velocityRectangleThickness;
            Vector2 pos = leftEdgeCenter(x, y);
            pos.y -= height / 2.0f;
            if (val < 0) pos.x += val;
            DrawRectangleV(pos, (Vector2){width, height}, RAYWHITE);
            DrawRectangleLinesEx((Rectangle){pos.x, pos.y, width, height}, 0.01f, BLACK);
        }
    }
}

void GridVisualization::drawVelY() {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y <= fluidGrid.cellCountY; y++) {
            float val = fluidGrid.velY[fluidGrid.idxY(x, y)] * halfCellSize;
            float height = fabsf(val);
            float width = halfCellSize * velocityRectangleThickness;
            Vector2 pos = bottomEdgeCenter(x, y);
            pos.x -= width / 2.0f;
            if (val < 0) pos.y += val;
            DrawRectangleV(pos, (Vector2){width, height}, RAYWHITE);
            DrawRectangleLinesEx((Rectangle){pos.x, pos.y, width, height}, 0.01f, BLACK);
        }
    }
}

void GridVisualization::drawDebugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback) {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 center = cellCenter(x, y);
            center.y *= -1.0f;
            Vector2 screenPos = GetWorldToScreen2D(center, camera);

            std::string text = callback(fluidGrid, x, y);
            int fontSize = 25;
            float textLen = (float)MeasureText(text.c_str(), fontSize);
            DrawOutlinedText(text.c_str(), (int)(screenPos.x - textLen / 2.0f), (int)(screenPos.y - fontSize / 2.0f), fontSize, WHITE, 3, BLACK);
        }
    }
}
