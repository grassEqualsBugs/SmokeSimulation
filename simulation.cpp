#include "include/raylib.h"
#include "include/rlgl.h"
#include "include/raymath.h"
#include <vector>
#include <string>
#include <random>
#include <functional>

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const char* WINDOW_TITLE = "";

// Purely computational, not a visualization
class FluidGrid {
public:
    int cellCountX;
    int cellCountY;
    int cellSize;
    std::vector<std::vector<float>> velX;
    std::vector<std::vector<float>> velY;

    void randomizeVelXY() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-1.f, 1.f);

        for (auto& row : velX)
            for (auto& v : row)
                v = dist(rng);

        for (auto& row : velY)
            for (auto& v : row)
                v = dist(rng);
    }

    FluidGrid(int cellCountX, int cellCountY, int cellSize)
        : cellCountX(cellCountX), cellCountY(cellCountY), cellSize(cellSize),
          velX(cellCountX + 1, std::vector<float>(cellCountY, 0.f)),
          velY(cellCountX, std::vector<float>(cellCountY + 1, 0.f)) {
        randomizeVelXY();
    }

    float calculateDivVelocityAtCell(int cellX, int cellY) {
        float top = velY[cellX][cellY+1];
        float bottom = velY[cellX][cellY];
        float left = velX[cellX][cellY];
        float right = velX[cellX+1][cellY];
        float gradX = (right - left) / cellSize;
        float gradY = (top - bottom) / cellSize;
        float div = gradX + gradY; // div(u) = ∂u/∂x + ∂u/∂y
        return div;
    }
};

// Grid visualization
class GridVisualization {
private:
    void DrawOutlinedText(const char *text, int posX, int posY, int fontSize, Color color, int outlineSize, Color outlineColor) {
        DrawText(text, posX - outlineSize, posY - outlineSize, fontSize, outlineColor);
        DrawText(text, posX + outlineSize, posY - outlineSize, fontSize, outlineColor);
        DrawText(text, posX - outlineSize, posY + outlineSize, fontSize, outlineColor);
        DrawText(text, posX + outlineSize, posY + outlineSize, fontSize, outlineColor);
        DrawText(text, posX, posY, fontSize, color);
    }
public:
    FluidGrid& fluidGrid;
    Vector2 cellDisplaySize;
    Vector2 boundsSize;
    Vector2 bottomLeft;
    float cellBorderThickness = 0.03;
    float halfCellSize;
    GridVisualization(FluidGrid& fluidGrid)
    : fluidGrid(fluidGrid) {
        cellDisplaySize = Vector2Scale(Vector2One(), fluidGrid.cellSize * (1 - cellBorderThickness));
        boundsSize = Vector2Scale((Vector2){(float)fluidGrid.cellCountX, (float)fluidGrid.cellCountY}, fluidGrid.cellSize);
        bottomLeft = Vector2Scale(boundsSize, -0.5f);
        halfCellSize = fluidGrid.cellSize * 0.5f;
    }

    Vector2 cellCenter(int x, int y) {
        return Vector2Add(bottomLeft, Vector2Scale((Vector2){x + 0.5f, y + 0.5f}, fluidGrid.cellSize));
    }

    Vector2 cellBottomLeft(int x, int y) {
        return Vector2Add(bottomLeft, Vector2Scale((Vector2){(float)x, (float)y}, fluidGrid.cellSize));
    }

    Vector2 leftEdgeCenter(int x, int y) {
        return Vector2Subtract(cellCenter(x, y), (Vector2){halfCellSize, 0.0f});
    }

    Vector2 bottomEdgeCenter(int x, int y) {
        return Vector2Subtract(cellCenter(x, y), (Vector2){0.0f, halfCellSize});
    }

    void renderGrid() {
        // Cells
        for (int x = 0; x < fluidGrid.cellCountX; x++) {
            for (int y = 0; y < fluidGrid.cellCountY; y++) {
                Vector2 center = cellCenter(x, y);
                Vector2 offset = Vector2Scale(cellDisplaySize, 0.5f);
                Vector2 pos = Vector2Subtract(center, offset);
                DrawRectangle(pos.x, pos.y, cellDisplaySize.x, cellDisplaySize.y, DARKGRAY);
            }
        }

        float rectangleScale = 0.2f;
        // X Velocity Rectangles
        for (int x = 0; x < fluidGrid.velX.size(); x++) {
            for (int y = 0; y < fluidGrid.velX[0].size(); y++) {
                float val = fluidGrid.velX[x][y] * halfCellSize;
                float width = fabsf(val);
                float height = halfCellSize * rectangleScale;
                Vector2 pos = leftEdgeCenter(x, y);
                pos.y -= height / 2.0f;
                if (val < 0) pos.x += val;
                DrawRectangleV(pos, (Vector2){width, height}, BLUE);
            }
        }

        // Y Velocity Rectangles
        for (int x = 0; x < fluidGrid.velY.size(); x++) {
            for (int y = 0; y < fluidGrid.velY[0].size(); y++) {
                float val = fluidGrid.velY[x][y] * halfCellSize;
                float height = fabsf(val);
                float width = halfCellSize * rectangleScale;
                Vector2 pos = bottomEdgeCenter(x, y);
                pos.x -= width / 2.0f;
                if (val < 0) pos.y += val;
                DrawRectangleV(pos, (Vector2){width, height}, RED);
            }
        }
    }

    void debugCellText(Camera2D camera, std::function<std::string(FluidGrid&, int, int)> callback) {
        for (int x = 0; x < fluidGrid.cellCountX; x++) {
            for (int y = 0; y < fluidGrid.cellCountY; y++) {
                Vector2 center = cellCenter(x, y);
                // Manually apply the world-space flip before projecting
                center.y *= -1.0f;
                Vector2 screenPos = GetWorldToScreen2D(center, camera);

                std::string text = callback(fluidGrid, x, y);
                int fontSize = 25;
                float textLength = MeasureText(text.c_str(), fontSize);
                DrawOutlinedText(text.c_str(), screenPos.x - textLength / 2.f, screenPos.y - fontSize / 2.f, fontSize, WHITE, 3, BLACK);
            }
        }
    }
};

int main() {
	SetConfigFlags(FLAG_VSYNC_HINT);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
	SetTargetFPS(60);

	Camera2D camera = {0};
	camera.offset = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
	camera.zoom = 1.f;

	FluidGrid fluidGrid(5, 3, 150);
	GridVisualization vis(fluidGrid);

	while (!WindowShouldClose()) {
	    if (IsKeyPressed(KEY_SPACE)) fluidGrid.randomizeVelXY();

		BeginDrawing();
		ClearBackground(BLACK);
		rlSetCullFace(RL_CULL_FACE_FRONT);
		BeginMode2D(camera);
		rlPushMatrix();
		rlScalef(1.0f, -1.0f, 1.0f);
		vis.renderGrid();
		rlPopMatrix();
		EndMode2D();
		vis.debugCellText(camera, [](FluidGrid& grid, int x, int y) {
			return std::to_string(grid.calculateDivVelocityAtCell(x, y)*100);
		});
		rlSetCullFace(RL_CULL_FACE_BACK);
		DrawFPS(10, 10);
		EndDrawing();
	}

	return 0;
}
