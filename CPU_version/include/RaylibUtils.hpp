#pragma once
#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace RaylibUtils {
    inline Vector4 ColorToVec4(Color c) {
        return (Vector4){(float)c.r, (float)c.g, (float)c.b, (float)c.a};
    }

    inline Color Vec4ToColor(Vector4 v) {
        return (Color){(unsigned char)v.x, (unsigned char)v.y, (unsigned char)v.z, (unsigned char)v.w};
    }

    inline void DrawOutlinedText(const char *text, int posX, int posY, int fontSize, Color color, int outlineSize, Color outlineColor) {
        DrawText(text, posX - outlineSize, posY - outlineSize, fontSize, outlineColor);
        DrawText(text, posX + outlineSize, posY - outlineSize, fontSize, outlineColor);
        DrawText(text, posX - outlineSize, posY + outlineSize, fontSize, outlineColor);
        DrawText(text, posX + outlineSize, posY + outlineSize, fontSize, outlineColor);
        DrawText(text, posX, posY, fontSize, color);
    }

    inline void DrawArrow(Vector2 start, Vector2 end, Color col, float thickness) {
        DrawLineEx(start, end, thickness, col);

        Vector2 delta = Vector2Subtract(end, start);
        float length = Vector2Length(delta);
        if (length < 0.0001f) return;

        float headSize = 0.33f * length;
        float angle = atan2f(delta.y, delta.x);

        Vector2 head1 = {
            end.x - headSize * cosf(angle - PI/12.0f),
            end.y - headSize * sinf(angle - PI/12.0f)
        };
        Vector2 head2 = {
            end.x - headSize * cosf(angle + PI/12.0f),
            end.y - headSize * sinf(angle + PI/12.0f)
        };

        float wingThickness = thickness * (2.0f / 3.0f);
        DrawLineEx(end, head1, wingThickness, col);
        DrawLineEx(end, head2, wingThickness, col);
    }
}
