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

private:
    Camera2D GridCamera = { 0 };
    float zoom = 1.0f;

    Color highlightColor = { 255, 255, 0, 80 };

    std::vector<std::string> tileSize = { "32x32", "64x64" };

    RenderTexture2D gridTexture;
    bool needsRedraw = true;
    bool initialized = false;
};
