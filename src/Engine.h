#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

#include "Grid.h"
#include "Entities/PlayerEntity.h"
#include "Managers/PlayerManager.h"
#include "Entities/EnemyEntity.h"
#include "Managers/EnemyManager.h"
#include "UI/UI.h"
#include "AssetManagement/AssetManager.h"
#include "Entities/TileMap.h"
#include "Scene/Scene.h"
#include "Entities/StaticEntity.h"
#include "Project/SaveLoad.h"
#include "Animation/Animation.h"
#include "Animation/Animator.h"
#include "Animation/AnimationEditor.h"
#include "AssetManagement/AssetImporter.h"
#include "AssetManagement/AssetDefinition.h"

class Engine {
public:
    enum class Mode { EDIT, PLAY };
    enum class ToolState { NONE, PLACING_PLAYER, PLACING_ENEMY, PLACING_ASSET, REMOVING_ASSET, REMOVING_PLAYER, REMOVING_ENEMY, PLACING_TILE, REMOVING_TILE };

    Engine();
    ~Engine();

    void Init();
    void Run();
    void Shutdown() const;

    Mode currentMode = Mode::EDIT;
    ToolState currentTool = ToolState::NONE;

    bool playModeWindowOpen = false;
    RenderTexture2D playModeTexture;

    static constexpr int MAX_LAYERS = 20;
    std::vector<bool> layerVisibility;

    // Layer management API
    bool IsLayerVisible(int layer) const;
    void SetLayerVisible(int layer, bool visible);
    void ToggleLayerVisibility(int layer);
    int GetTotalLayers() const;
    void SetTotalLayers(int count);
    void AddLayer();
    void RemoveLayer();
    bool CanRemoveLayer() const;
    void SetCurrentTileLayer(int layer);
    int GetCurrentTileLayer() const;
    void CycleLayerUp();
    void CycleLayerDown();
    bool CanRemoveSpecificLayer(int layerIndex) const;
    void RemoveSpecificLayer(int layerIndex);

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
        int activeLayer = 0;
        int totalLayers = 5;
    } tileToolState;

    struct TileRef {
        std::string assetID;
        int tileIndex = 0;
    };

    struct AnimatedTileFrame {
        std::string assetID;
        int tileIndex = 0;
        float duration = 0.2f;
    };

    struct AnimatedTileDefinition {
        std::string id;
        bool loop = true;
        std::vector<TileRef> baseTiles;
        std::vector<AnimatedTileFrame> frames;
    };

    struct {
        Asset* asset = nullptr;
        bool isPlacing = false;
        bool showPreview = false;
        Vector2 previewPosition = {0, 0};
        float previewAlpha = 0.5f;
    } assetToolState;

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

    std::unique_ptr<TileMap>& GetPlayModeTileMap() {
        return playModeTileMap;
    }

    const std::vector<CameraResolution>& GetAvailableResolutions() const {
        return availableResolutions;
    }

    int GetSelectedResolutionIndex() const {
        return selectedResolutionIndex;
    }

    Grid& GetGrid() {
        return grid;
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
        return static_cast<int>(scenes.size());
    }

    void SetCurrentSceneByIndex(const int value) {
        if (value >= 0 && value < scenes.size()) {
            auto it = scenes.begin();
            std::advance(it, value);
            currentSceneID = it->first;
        }
    }

    int GetCurrentSceneIndex() const {
        int index = 0;
        for (const auto&[fst, snd] : scenes) {
            if (fst == currentSceneID) {
                return index;
            }
            ++index;
        }
        return -1; // Not found
    }
    void SetCurrentSceneIndex(const int index) {
        if (index >= 0 && index < scenes.size()) {
            auto it = scenes.begin();
            std::advance(it, index);
            currentSceneID = it->first;
        }
    }
    std::string GetSceneName(const int index) const {
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
    void RenameScene(const int index, const std::string& newName) {
        if (index >= 0 && index < scenes.size()) {
            auto it = scenes.begin();
            std::advance(it, index);
            it->second->SetName(newName);
        }
    }

    std::string GetSceneID(const int index) const {
        if (index < 0 || index >= static_cast<int>(scenes.size())) {
            return "";
        }
        auto it = scenes.begin();
        std::advance(it, index);
        return it->first;
    }

    std::string GetStartingSceneID() const {
        return startingSceneID;
    }
    void SetStartingSceneID(const std::string &ID) {
        startingSceneID = ID;
    }

    void SetNextSceneId(const int value) {
        nextSceneId = value;
    }

    int GetNextSceneId() const {
        return nextSceneId;
    }

    void CreateNewScene() {
        HandleSceneCreation();
    }

    void DeleteScene(const int& index) {
        const int& sceneIndex = index;
        HandleSceneDeletion(sceneIndex);
    }

    AssetImporter& GetAssetImporter() {
        return assetImporter;
    }

    AnimationEditor& GetAnimationEditor() {
        return animationEditor;
    }

    PlayerManager &GetPlayerManager() {
        return playerManager;
    }

    EnemyManager &GetEnemyManager() {
        return enemyManager;
    }

    const std::vector<AnimatedTileDefinition>& GetAnimatedTileDefinitions() const {
        return animatedTileDefinitions;
    }

    void SetAnimatedTileDefinitions(const std::vector<AnimatedTileDefinition>& definitions);
    void ClearAnimatedTileDefinitions();
    void SetAnimatedTileClock(float seconds);
    float GetAnimatedTileClock() const;

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
    std::string startingSceneID;
    AnimationSet testAnimationSet;
    Animator testAnimator;
    Vector2 testAnimatorPosition = {200, 200};
    AnimationEditor animationEditor;
    AssetImporter assetImporter;
    std::vector<AnimatedTileDefinition> animatedTileDefinitions;
    std::unordered_map<std::string, size_t> animatedTileLookup;
    float animatedTileClockSeconds = 0.0f;

    int lastPlacedTileX;
    int lastPlacedTileY;
    bool isPlacingTiles;

    Camera2D playerCamera;
    Rectangle playerCameraArea = {0, 0, 0, 0};

    // Input Handling
    void HandlePlayerPlacement();
    void HandlePlayerRemoval();
    void HandleEnemyPlacement();
    void HandleEnemyRemoval();
    void HandleTilePlacement();
    void HandleTileRemoval();
    void DrawHoveredTileLayerInfo();
    void HandleAssetPlacement();
    void HandleAssetRemoval();
    void UpdateAssetPlacementPreview();
    void DrawAssetPlacementPreview() const;
    void HandleSceneCreation();
    void HandleSceneDeletion(const int& index);
    void ChangeScene(const std::string& targetSceneID, int spawnGridX, int spawnGridY);
    void HandleSceneSwitchInPlayMode();
    void DrawEditModeTiles();
    void DrawPlayModeTiles();
    bool ResolveAnimatedTileFrame(const TileData& tile, const Asset*& outAsset, Rectangle& outSourceRect);
    void RebuildAnimatedTileLookup();
    std::string BuildAnimatedTileKey(const std::string& assetID, int tileIndex) const;
    void HandleEditModeInput();
    bool IsMouseOverUI() const;

    // Helper methods
    void CreatePlayModeSnapshots();
    void UpdatePlayModeCamera();
    void UpdateEditModeCamera();
    void LoadAssets();
    void ResolveCollisionInPlayMode();
};
