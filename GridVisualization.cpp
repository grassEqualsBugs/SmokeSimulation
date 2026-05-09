#include "include/GridVisualization.hpp"
#include "include/rlgl.h"
#include "include/raymath.h"
#include <cmath>

GridVisualization::GridVisualization(FluidGrid& fluidGrid)
    : fluidGrid(fluidGrid) {
    cellDisplaySize = Vector2Scale(Vector2One(), fluidGrid.cellSize * (1 - cellBorderThickness));
    boundsSize = Vector2Scale((Vector2){(float)fluidGrid.cellCountX, (float)fluidGrid.cellCountY}, (float)fluidGrid.cellSize);
    bottomLeft = Vector2Scale(boundsSize, -0.5f);
    halfCellSize = fluidGrid.cellSize * 0.5f;
}

void GridVisualization::DrawOutlinedText(const char *text, int posX, int posY, int fontSize, Color color, int outlineSize, Color outlineColor) {
    DrawText(text, posX - outlineSize, posY - outlineSize, fontSize, outlineColor);
    DrawText(text, posX + outlineSize, posY - outlineSize, fontSize, outlineColor);
    DrawText(text, posX - outlineSize, posY + outlineSize, fontSize, outlineColor);
    DrawText(text, posX + outlineSize, posY + outlineSize, fontSize, outlineColor);
    DrawText(text, posX, posY, fontSize, color);
}

Vector2 GridVisualization::cellCenter(int x, int y) {
    return Vector2Add(bottomLeft, Vector2Scale((Vector2){x + 0.5f, y + 0.5f}, (float)fluidGrid.cellSize));
}

Vector2 GridVisualization::cellBottomLeft(int x, int y) {
    return Vector2Add(bottomLeft, Vector2Scale((Vector2){(float)x, (float)y}, (float)fluidGrid.cellSize));
}

Vector2 GridVisualization::leftEdgeCenter(int x, int y) {
    return Vector2Subtract(cellCenter(x, y), (Vector2){halfCellSize, 0.0f});
}

Vector2 GridVisualization::bottomEdgeCenter(int x, int y) {
    return Vector2Subtract(cellCenter(x, y), (Vector2){0.0f, halfCellSize});
}

void GridVisualization::renderGrid() {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 center = cellCenter(x, y);
            Vector2 offset = Vector2Scale(cellDisplaySize, 0.5f);
            Vector2 pos = Vector2Subtract(center, offset);
            Color col = fluidGrid.isSolid(x,y) ? (Color){40, 40, 40, 255} : DARKGRAY;
            DrawRectangleV(pos, cellDisplaySize, col);
        }
    }

    for (int x = 0; x < (int)fluidGrid.velX.size(); x++) {
        for (int y = 0; y < (int)fluidGrid.velX[0].size(); y++) {
            float val = fluidGrid.velX[x][y] * halfCellSize;
            float width = fabsf(val);
            float height = halfCellSize * velocityRectangleThickness;
            Vector2 pos = leftEdgeCenter(x, y);
            pos.y -= height / 2.0f;
            if (val < 0) pos.x += val;
            DrawRectangleV(pos, (Vector2){width, height}, DARKBLUE);
        }
    }

    for (int x = 0; x < (int)fluidGrid.velY.size(); x++) {
        for (int y = 0; y < (int)fluidGrid.velY[0].size(); y++) {
            float val = fluidGrid.velY[x][y] * halfCellSize;
            float height = fabsf(val);
            float width = halfCellSize * velocityRectangleThickness;
            Vector2 pos = bottomEdgeCenter(x, y);
            pos.x -= width / 2.0f;
            if (val < 0) pos.y += val;
            DrawRectangleV(pos, (Vector2){width, height}, DARKBLUE);
        }
    }
}

void GridVisualization::debugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback) {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 center = cellCenter(x, y);
            center.y *= -1.0f;
            Vector2 screenPos = GetWorldToScreen2D(center, camera);

            std::string text = callback(fluidGrid, x, y);
            int fontSize = 25;
            float textLength = (float)MeasureText(text.c_str(), fontSize);
            DrawOutlinedText(text.c_str(), (int)(screenPos.x - textLength / 2.f), (int)(screenPos.y - fontSize / 2.f), fontSize, WHITE, 3, BLACK);
        }
    }
}
