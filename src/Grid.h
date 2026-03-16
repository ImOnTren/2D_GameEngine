#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "raymath.h"

class Grid {
public:
    Grid();
    ~Grid();

    void Update();
    void Draw();

    int GetTileSize() const {
        return 32;
    }

    Camera2D GetCamera() const {
        return GridCamera;
    }

    bool IsValidCell(int gridX, int gridY) const {
        return gridX >= 0 && gridY >= 0;
    }

    bool IsValidWorldPosition(Vector2 worldPos) const {
        const int tileSize = GetTileSize();
        const int gridX = static_cast<int>(worldPos.x) / tileSize;
        const int gridY = static_cast<int>(worldPos.y) / tileSize;
        return IsValidCell(gridX, gridY);
    }

private:
    Camera2D GridCamera = { 0 };
    float zoom = 1.0f;

    Color highlightColor = { 255, 255, 0, 80 };

    std::vector<std::string> tileSize = { "32x32", "64x64" };

    RenderTexture2D gridTexture;
    bool needsRedraw = true;
    bool initialized = false;
};
