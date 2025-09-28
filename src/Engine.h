// Engine.h (updated)
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "Grid.h"
#include "Entities/PlayerEntity.h"
#include "Entities/PlayerManager.h"
#include "Entities/EnemyEntity.h"
#include "Entities/EnemyManager.h"
#include "UI/UI.h"

class Engine {
public:
    enum class Mode { EDIT, PLAY };
    enum class ToolState { NONE, PLACING_PLAYER, PLACING_ENEMY, REMOVING_PLAYER, REMOVING_ENEMY };
    // Removed CAMERA_SELECTION

    Engine();
    ~Engine();

    void Init();
    void Run();
    void Shutdown();

    Mode currentMode = Mode::EDIT;
    ToolState currentTool = ToolState::NONE;
    bool playModeWindowOpen = false;
    RenderTexture2D playModeTexture;

    // Camera resolution settings
    struct CameraResolution {
        int width;
        int height;
        const char* name;
    };

    std::vector<CameraResolution> availableResolutions = {
        {640, 360, "640x360 (16:9)"},
        {800, 450, "800x450 (16:9)"},
        {960, 540, "960x540 (16:9)"},
        {1280, 720, "1280x720 (16:9)"},
        {854, 480, "854x480 (16:9)"},
        {426, 240, "426x240 (16:9)"},
        {800, 600, "800x600 (4:3)"},
        {1024, 768, "1024x768 (4:3)"}
    };
    int selectedResolutionIndex = 0;

    void ResetTool() { currentTool = ToolState::NONE; }
    void UpdatePlayerCamera();

    bool HasPlayer() const {
        return editModePlayer != nullptr;
    }

    const std::vector<CameraResolution>& GetAvailableResolutions() const {
        return availableResolutions;
    }

    int GetSelectedResolutionIndex() const {
        return selectedResolutionIndex;
    }

    void SetSelectedResolutionIndex(int index) {
        if (index >= 0 && index < availableResolutions.size()) {
            selectedResolutionIndex = index;
            // Update the appropriate camera based on current mode
            if (currentMode == Mode::PLAY) {
                UpdatePlayModeCamera();
            } else {
                UpdateEditModeCamera();
            }
        }
    }

    void StartPlayMode();
    void StopPlayMode();

private:
    Grid grid;
    PlayerManager playerManager;
    EnemyManager enemyManager;

    // Edit mode entities (preserved when switching to play mode)
    std::vector<std::unique_ptr<EnemyEntity>> editModeEnemies;
    std::unique_ptr<PlayerEntity> editModePlayer;

    // Play mode entities (copies for simulation)
    std::vector<std::unique_ptr<EnemyEntity>> playModeEnemies;
    std::unique_ptr<PlayerEntity> playModePlayer;

    // Separate camera areas for edit mode and play mode
    Rectangle editModeCameraArea = {0, 0, 0, 0};  // Frozen during play mode
    Rectangle playModeCameraArea = {0, 0, 0, 0};  // Updated during play mode

    Camera2D playerCamera;
    Rectangle playerCameraArea = {0, 0, 0, 0};

    // Input Handling
    void HandlePlayerPlacement();
    void HandlePlayerRemoval();
    void HandleEnemyPlacement();
    void HandleEnemyRemoval();
    void HandleEditModeInput();
    bool IsMouseOverUI() const;

    // methods for play mode management
    void CopyEditToPlayMode();
    void UpdateEditModeCamera();  // For edit mode camera area
    void UpdatePlayModeCamera();  // For play mode camera area
};