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
#include "Managers/PlayerManager.h"
#include "Entities/EnemyEntity.h"
#include "Managers/EnemyManager.h"
#include "UI/UI.h"
#include "Managers/AssetManager.h"
#include "Entities/TileMap.h"
#include "Scene/Scene.h"

class Engine {
public:
    enum class Mode { EDIT, PLAY };
    enum class ToolState { NONE, PLACING_PLAYER, PLACING_ENEMY, REMOVING_PLAYER, REMOVING_ENEMY, PLACING_TILE, REMOVING_TILE };

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

    struct {
        Asset* tileset = nullptr;
        int selectedTileIndex = -1;
        bool isPlacingTile = false;
    } tileToolState;

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

    bool HasPlayer();

    const std::vector<CameraResolution>& GetAvailableResolutions() const {
        return availableResolutions;
    }

    int GetSelectedResolutionIndex() const {
        return selectedResolutionIndex;
    }

    void SetSelectedResolutionIndex(int index) {
        if (index >= 0 && index < availableResolutions.size()) {
            selectedResolutionIndex = index;
            // Update the appropriate camera based on the current mode
            if (currentMode == Mode::PLAY) {
                UpdatePlayModeCamera();
            } else {
                UpdateEditModeCamera();
            }
        }
    }

    int GetEnemyCount();

    AssetManager& GetAssetManager(){
        return assetManager;
    }

    // Scene management API
    std::unordered_map<std::string, std::unique_ptr<Scene>>& GetAllScenes() {
        return scenes;
    }
    Scene* GetCurrentScene() {
        if (scenes.find(currentSceneID) != scenes.end()) {
            return scenes[currentSceneID].get();
        }
        return nullptr;
    }
    void SetCurrentScene(const std::string& sceneID) {
        if (scenes.find(sceneID) != scenes.end()) {
            currentSceneID = sceneID;
        }
    }
    int GetSceneCount() const {
        return scenes.size();
    }
    int GetCurrentSceneIndex() const {
        int index = 0;
        for (const auto& pair : scenes) {
            if (pair.first == currentSceneID) {
                return index;
            }
            ++index;
        }
        return -1; // Not found
    }
    void SetCurrentSceneIndex(int index) {
        if (index >= 0 && index < scenes.size()) {
            auto it = scenes.begin();
            std::advance(it, index);
            currentSceneID = it->first;
        }
    }
    std::string GetSceneName(int index) const {
        if (index >= 0 && index < scenes.size()) {
            auto it = scenes.begin();
            std::advance(it, index);
            return it->second->GetName();
        }
        return "";
    }
    Rectangle GetSceneCameraArea()
    {
        return GetCurrentScene()->GetEditModeCameraArea();
    }
    void RenameScene(int index, const std::string& newName) {
        if (index >= 0 && index < scenes.size()) {
            auto it = scenes.begin();
            std::advance(it, index);
            it->second->SetName(newName);
        }
    }
    int  GetStartingSceneIndex() const {
        return startingSceneIndex;
    }
    void SetStartingSceneIndex(int index) {
        startingSceneIndex = index;
    }

    void CreateNewScene() {
        HandleSceneCreation();
    }
    void DeleteScene(const int& index) {
        const int& sceneIndex = index;
        HandleSceneDeletion(sceneIndex);
    }

    void StartPlayMode();
    void StopPlayMode();

private:
    Grid grid;
    PlayerManager playerManager;
    EnemyManager enemyManager;
    AssetManager assetManager;
    std::unique_ptr<TileMap> playModeTileMap;
    std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
    std::string currentSceneID;
    int nextSceneId = 1;
    int startingSceneIndex = -1;

    Camera2D playerCamera;
    Rectangle playerCameraArea = {0, 0, 0, 0};

    // Input Handling
    void HandlePlayerPlacement();
    void HandlePlayerRemoval();
    void HandleEnemyPlacement();
    void HandleEnemyRemoval();
    void HandleTilePlacement();
    void HandleTileRemoval();
    void HandleSceneCreation();
    void HandleSceneDeletion(const int& index);
    void DrawEditModeTiles();
    void DrawPlayModeTiles();
    void HandleEditModeInput();
    bool IsMouseOverUI() const;

    // Helper methods
    void CreatePlayModeSnapshots();
    void UpdatePlayModeCamera();
    void UpdateEditModeCamera();
    void LoadAssets();
};