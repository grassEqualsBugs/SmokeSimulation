#include "include/raylib.h"
#include "include/rlgl.h"
#include "include/FluidGrid.hpp"
#include "include/GridVisualization.hpp"
#include "include/Brush.hpp"

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const char* WINDOW_TITLE = "Smoke Simulation";

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    Camera2D camera = {0};
    camera.offset = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    camera.zoom = 100.0f;

    FluidConfig config;
    config.cellSize = 0.1f;
    config.pressureIterations = 200;
    config.deltaTime = 1 / 30.f;
    config.density = 1.f;

    FluidGrid fluidGrid(160, 90, config);
    GridVisualization vis(fluidGrid, 1, GridVisMode::SMOKE);
    Brush brush(fluidGrid, 0.9f, 1.8f);

    bool paused = false;

    while (!WindowShouldClose()) {
        // Key Inputs
        if (IsKeyPressed(KEY_R)) fluidGrid.reset();
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyPressed(KEY_ONE)) vis.visMode = GridVisMode::SMOKE;
        if (IsKeyPressed(KEY_TWO)) vis.visMode = GridVisMode::SPEED;
        if (IsKeyPressed(KEY_THREE)) vis.visMode = GridVisMode::DIVERGENCE;

        // Updating
        brush.update(camera);
        if (!paused) fluidGrid.update();

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

        rlSetCullFace(RL_CULL_FACE_BACK);
        DrawText((std::to_string(GetFPS()).c_str()), 10, 10, 20, RAYWHITE);
        if (vis.visMode == GridVisMode::DIVERGENCE) {
            DrawText(("ERROR: " + std::to_string(fluidGrid.calculateDivergenceError())).c_str(), 10, 30, 20, RAYWHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
