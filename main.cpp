#include "include/raylib.h"
#include "include/rlgl.h"
#include "include/FluidGrid.hpp"
#include "include/GridVisualization.hpp"
#include "include/VelocityBrush.hpp"

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const char* WINDOW_TITLE = "Fluid Simulation";

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    Camera2D camera = {0};
    camera.offset = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    camera.zoom = 200.0f;

    FluidConfig config;
    config.cellSize = 0.5f;
    config.pressureIterations = 10;

    FluidGrid fluidGrid(16, 9, config);
    GridVisualization vis(fluidGrid, 4);
    VelocityBrush brush(fluidGrid, 0.5f);

    bool isSolverOn = false;

    while (!WindowShouldClose()) {
        // Key Inputs
        if (IsKeyPressed(KEY_R)) fluidGrid.reset();
        if (IsKeyPressed(KEY_S)) isSolverOn = !isSolverOn;

        // Updating
        brush.update(camera);
        if (isSolverOn) fluidGrid.update();

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        rlSetCullFace(RL_CULL_FACE_FRONT);
        BeginMode2D(camera);
            rlPushMatrix();
                rlScalef(1.0f, -1.0f, 1.0f);
                vis.renderGrid();
                brush.render(camera);
            rlPopMatrix();
        EndMode2D();

        // vis.drawDebugCellText(camera, [](FluidGrid& grid, int x, int y) {
        //     return std::string(TextFormat("%.2f", grid.calculateDivVelocityAtCell(x, y)));
        // });

        rlSetCullFace(RL_CULL_FACE_BACK);
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
