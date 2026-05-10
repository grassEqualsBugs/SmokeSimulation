#include "include/GridVisualization.hpp"
#include "include/RaylibUtils.hpp"
#include "include/rlgl.h"
#include "include/raymath.h"
#include <cmath>

using namespace RaylibUtils;

GridVisualization::GridVisualization(FluidGrid& fluidGrid, int interpolatedVelocitiesPerSide)
    : fluidGrid(fluidGrid), interpolatedVelocitiesPerSide(interpolatedVelocitiesPerSide) {
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
    // drawVelX();
    // drawVelY();
    drawInterpolatedVelocities();
}

void GridVisualization::drawCells() {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 center = cellCenter(x, y);
            Vector2 offset = Vector2Scale(cellDisplaySize, 0.5f);
            Vector2 pos = Vector2Subtract(center, offset);

            Color col;
            if (fluidGrid.isSolid(x, y)) {
                col = (Color){10, 10, 10, 255};
            } else {
                float div = fluidGrid.calculateDivVelocityAtCell(x, y);
                float t = fminf(fabsf(div) / divergenceColorRange, 1.0f);
                Color target = (div < 0) ? Color{245, 66, 66, 255} : Color{66, 135, 245, 255};
                col = Vec4ToColor(Vector4Lerp(ColorToVec4(Color{30, 30, 30, 255}), ColorToVec4(target), t));
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

void GridVisualization::drawInterpolatedVelocities() {
    if (interpolatedVelocitiesPerSide <= 0) return;

    float h = fluidGrid.config.cellSize;
    float subH = h / (float)interpolatedVelocitiesPerSide;

    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            if (fluidGrid.isSolid(x, y)) continue;

            Vector2 bl = cellBottomLeft(x, y);

            for (int i = 0; i < interpolatedVelocitiesPerSide; i++) {
                for (int j = 0; j < interpolatedVelocitiesPerSide; j++) {
                    Vector2 pos = {
                        bl.x + (i + 0.5f) * subH,
                        bl.y + (j + 0.5f) * subH
                    };

                    Vector2 vel = fluidGrid.getVelocityAtWorldPos(pos);
                    float speed = Vector2Length(vel);
                    if (speed < 0.001f) continue;

                    Vector2 end = Vector2Add(pos, Vector2Scale(vel, subH * 2.f));

                    // Draw the main line (arrow shaft)
                    Color arrowColor = (Color){150, 191, 255, 255};
                    DrawLineEx(pos, end, 0.012f, arrowColor);

                    // Draw the arrow head
                    float headSize = 0.33 * Vector2Length(Vector2Subtract(end, pos));
                    float angle = atan2f(vel.y, vel.x);
                    Vector2 head1 = {
                        end.x - headSize * cosf(angle - PI/12),
                        end.y - headSize * sinf(angle - PI/12)
                    };
                    Vector2 head2 = {
                        end.x - headSize * cosf(angle + PI/12),
                        end.y - headSize * sinf(angle + PI/12)
                    };
                    DrawLineEx(end, head1, 0.008f, arrowColor);
                    DrawLineEx(end, head2, 0.008f, arrowColor);
                }
            }
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
