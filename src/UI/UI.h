#pragma once

#include <string>
#include <vector>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

class Engine;
class Grid;

class UI {
public:
    static void RenderControlPanel(Engine& engine, Grid& grid);
    static void RenderControlConsole();
    static void RenderPlayModeWindow(Engine& engine);
    static void RenderAssetConsole();

    static void SetDebugMessage(const std::string& text) {
        DebugMessages.push_back(text);
        if (DebugMessages.size() > 20) {
            DebugMessages.erase(DebugMessages.begin());
        }
    };
    static std::vector<std::string> GetDebugMessages() {
        return DebugMessages;
    }
    static void ClearDebugMessages() {
        DebugMessages.clear();
    }

private:
    static void RenderModeControls(Engine& engine);
    static void RenderCameraResolutionControls(Engine& engine);
    static void RenderGridSizeControls(Grid& grid);
    static std::vector<std::string> DebugMessages;
};