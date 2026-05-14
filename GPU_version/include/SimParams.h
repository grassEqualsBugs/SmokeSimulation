#pragma once

#ifdef __METAL_VERSION__
#include <metal_stdlib>
using namespace metal;
#else
#include <simd/simd.h>
#include <stdint.h>
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
	simd_float2 mousePos;
	uint32_t lastMouseEvent;
} FrameData;

// NSEventType constants defined for shared use
enum MouseEventType {
    MouseEventTypeLeftMouseDown = 1,
    MouseEventTypeLeftMouseUp = 2,
    MouseEventTypeRightMouseDown = 3,
    MouseEventTypeRightMouseUp = 4,
    MouseEventTypeMouseMoved = 5,
    MouseEventTypeLeftMouseDragged = 6
};
