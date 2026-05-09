#pragma once
#include "raylib.h"
#include "raymath.h"

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
}
