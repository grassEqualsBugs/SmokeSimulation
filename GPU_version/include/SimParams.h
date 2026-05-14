#pragma once

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
    using namespace metal;
    typedef float2 SimFloat2;
#else
    #include <simd/simd.h>
    #include <stdint.h>
    typedef simd_float2 SimFloat2;
#endif

typedef struct {
    float cellSize;
    float deltaTime;
    float fluidDensity;
    int width;
    int height;
    float mouseRadius;
} SimConstants;

typedef struct {
    SimFloat2 pos;
    bool leftDown;
    bool rightDown;
} MouseState;

typedef struct {
    MouseState mouse;
} FrameData;
