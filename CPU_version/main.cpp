#include "include/raylib.h"
#include "include/rlgl.h"
#include "include/FluidGrid.hpp"
#include "include/GridVisualization.hpp"
#include "include/Brush.hpp"

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const char* WINDOW_TITLE = "Fluid Simulation";

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    Camera2D camera = {0};
    camera.offset = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    camera.zoom = 100.0f;

    FluidConfig config;
    config.cellSize = 0.2f;
    config.pressureIterations = 20;
    config.deltaTime = 1 / 30.f;

    FluidGrid fluidGrid(80, 45, config);
    GridVisualization vis(fluidGrid, 1, GridVisMode::SMOKE);
    Brush brush(fluidGrid, 0.6f, 2.0f);

    while (!WindowShouldClose()) {
        // Key Inputs
        if (IsKeyPressed(KEY_R)) fluidGrid.reset();

        // Updating
        brush.update(camera);
        fluidGrid.update();

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
        // DrawText(("ERROR: " + std::to_string(fluidGrid.calculateDivergenceError())).c_str(), 10, 30, 20, (Color){ 0, 228, 48, 255 });
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
