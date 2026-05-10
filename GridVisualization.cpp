#include "include/GridVisualization.hpp"
#include "include/RaylibUtils.hpp"
#include "include/rlgl.h"
#include "include/raymath.h"
#include <cmath>

using namespace RaylibUtils;

GridVisualization::GridVisualization(FluidGrid& fluidGrid, int interpolatedVelocitiesPerSide, GridVisMode visMode)
    : fluidGrid(fluidGrid), interpolatedVelocitiesPerSide(interpolatedVelocitiesPerSide), visMode(visMode) {
    const float h = fluidGrid.config.cellSize;
    cellDisplaySize = Vector2Scale(Vector2One(), h * (1.0f - cellBorderThickness));
    boundsSize = (Vector2){ (float)fluidGrid.cellCountX * h, (float)fluidGrid.cellCountY * h };
    halfCellSize = h * 0.5f;
}

void GridVisualization::renderGrid() {
    drawCells();
    drawInterpolatedVelocities();
    // drawVelX();
    // drawVelY();
}

void GridVisualization::drawCells() {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 cellPos = fluidGrid.cellCenter(x, y);
            Vector2 offset = Vector2Scale(cellDisplaySize, 0.5f);
            Vector2 pos = Vector2Subtract(cellPos, offset);

            Color col;
            if (fluidGrid.isSolid(x, y)) {
                col = (Color){10, 10, 10, 255};
            } else if (visMode == GridVisMode::DIVERGENCE) {
                float div = fluidGrid.calculateDivVelocityAtCell(x, y);
                float t = fminf(fabsf(div) / divergenceColorRange, 1.0f);
                Color target = (div < 0) ? Color{245, 66, 66, 255} : Color{66, 135, 245, 255};
                col = Vec4ToColor(Vector4Lerp(ColorToVec4(Color{30, 30, 30, 255}), ColorToVec4(target), t));
            } else if (visMode == GridVisMode::SPEED) {
                Vector2 velocity = fluidGrid.getVelocityAtWorldPos(cellPos);
                float speedT = fminf(Vector2Length(velocity) / speedVisMax, 1.0f);
                float hue = (1.0f - speedT) * 218.0f + speedT * 10.0f;
                col = ColorFromHSV(hue, 0.7f, 0.8f);
            }
            DrawRectangleV(pos, cellDisplaySize, col);
        }
    }
}

void GridVisualization::drawVelX() {
    for (int x = 0; x <= fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            float val = fluidGrid.velX[fluidGrid.idxX(x, y)];
            if (fabsf(val) < 0.001f) continue;

            Vector2 start = fluidGrid.leftEdgeCenter(x, y);
            Vector2 end = Vector2Add(start, (Vector2){ val * halfCellSize, 0.0f });
            DrawArrow(start, end, RAYWHITE, velocityArrowThickness);
        }
    }
}

void GridVisualization::drawVelY() {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y <= fluidGrid.cellCountY; y++) {
            float val = fluidGrid.velY[fluidGrid.idxY(x, y)];
            if (fabsf(val) < 0.001f) continue;

            Vector2 start = fluidGrid.bottomEdgeCenter(x, y);
            Vector2 end = Vector2Add(start, (Vector2){ 0.0f, val * halfCellSize });
            DrawArrow(start, end, RAYWHITE, velocityArrowThickness);
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

            Vector2 bl = fluidGrid.cellBottomLeft(x, y);

            for (int i = 0; i < interpolatedVelocitiesPerSide; i++) {
                for (int j = 0; j < interpolatedVelocitiesPerSide; j++) {
                    Vector2 pos = {
                        bl.x + (i + 0.5f) * subH,
                        bl.y + (j + 0.5f) * subH
                    };

                    Vector2 vel = fluidGrid.getVelocityAtWorldPos(pos);
                    float speed = Vector2Length(vel);
                    if (speed < 0.001f) continue;

                    Vector2 end = Vector2Add(pos, Vector2Scale(vel, subH));
                    DrawArrow(pos, end, BLACK, interpolatedVelocityArrowThickness);
                }
            }
        }
    }
}

void GridVisualization::drawDebugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback) {
    for (int x = 0; x < fluidGrid.cellCountX; x++) {
        for (int y = 0; y < fluidGrid.cellCountY; y++) {
            Vector2 center = fluidGrid.cellCenter(x, y);
            center.y *= -1.0f;
            Vector2 screenPos = GetWorldToScreen2D(center, camera);

            std::string text = callback(fluidGrid, x, y);
            int fontSize = 25;
            float textLen = (float)MeasureText(text.c_str(), fontSize);
            DrawOutlinedText(text.c_str(), (int)(screenPos.x - textLen / 2.0f), (int)(screenPos.y - fontSize / 2.0f), fontSize, WHITE, 3, BLACK);
        }
    }
}
