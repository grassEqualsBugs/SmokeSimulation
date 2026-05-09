#include "include/raylib.h"
#include "include/rlgl.h"
#include "include/FluidGrid.hpp"
#include "include/GridVisualization.hpp"

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const char* WINDOW_TITLE = "Fluid Simulation";

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    // SetTargetFPS(60);

    Camera2D camera = {0};
    camera.offset = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    camera.zoom = 200.0f;

    FluidConfig config;
    config.cellSize = 0.68f;
    config.pressureIterations = 1;

    FluidGrid fluidGrid(10, 6, config);
    GridVisualization vis(fluidGrid);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) fluidGrid.randomizeVelXY();
        if (IsKeyDown(KEY_S)) {
            fluidGrid.solvePressure();
            fluidGrid.updateVelocities();
        }

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
            return std::string(TextFormat("%.2f", grid.calculateDivVelocityAtCell(x, y)));
        });

        rlSetCullFace(RL_CULL_FACE_BACK);
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
