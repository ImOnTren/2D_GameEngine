#pragma once

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

class Engine;
class Grid;

class UI {
public:
    static void RenderControlPanel(Engine& engine, Grid& grid);
    static void RenderPlayModeWindow(Engine& engine);

private:
    static void RenderModeControls(Engine& engine);
    static void RenderCameraResolutionControls(Engine& engine);
    static void RenderGridSizeControls(Grid& grid);
};