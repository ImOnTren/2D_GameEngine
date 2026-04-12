#pragma once

#include <string>
#include <vector>
#include <format>
#include <iostream>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "AssetManagement/AssetManager.h"

class Entity;
class StaticEntity;
class PlayerEntity;
class EnemyEntity;
class Engine;
class Grid;

class UI {
public:
    static void RenderControlPanel(Engine& engine, const Grid& grid);
    static void RenderDebugConsole();
    static void RenderAssetConsole(Engine& engine);
    static void RenderPlayModeWindow(Engine& engine);
    static void RenderNewSceneTab(Engine& engine);

    static std::string GetAssetTypeName(AssetType type);

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

    static void ClearSelectedEntity();

    static float controlPanelWidthRatio;
    static float assetConsoleHeightRatio;

private:
    struct TutorialState {
        bool isOpen = false;
        int currentStep = 0;
    };

    static void RenderTutorialWindow();
    static void RenderModeControls(Engine& engine);
    static void RenderCameraResolutionControls(Engine& engine);
    static void RenderAssetFilters(AssetManager& assetManager, const std::vector<std::string>& categories);
    static void RenderCategoryList(const std::vector<std::string>& categories, std::vector<Asset*>& allAssets);
    static void RenderAssetGrid(Engine& engine, AssetManager& assetManager, const std::vector<Asset*>& allAssets);
    static void RenderAssetThumbnail(Asset* asset);
    static void RenderAssetDetails(Engine& engine, Asset* asset, AssetManager& assetManager);
    static void RenderTilesetPreview(const Asset* asset);
    static void RenderTileSelectionWindow(Engine& engine, Asset* tileset);
    static void RenderTilesetGrid(Engine& engine, Asset* tileset);
    static void RenderTileSceneContext(Engine& engine, const Grid& grid);
    static void RenderLayerVisibilityControls(Engine& engine);
    [[nodiscard]] static bool IsMouseOverUI();
    static std::vector<std::string> DebugMessages;

    static std::string selectedCategory;
    static std::string searchFilter;
    static AssetType typeFilter;
    static Asset* selectedAsset;
    static int thumbnailSize;
    static bool showTilesetPreview;
    static char searchBuffer[256];
    static Entity* selectedSceneEntity;

    static bool showTileSelectionWindow;
    static Asset* selectedTileset;
    static int selectedTileIndex;
    static Vector2 selectedTileCoords;
    static int tilesetTileWidth;
    static int tilesetTileHeight;
    static bool tileMapModified;
    static bool bulkEditActive;
    static int bulkEditStartX;
    static int bulkEditStartY;
    static int bulkEditEndX;
    static int bulkEditEndY;
    static TutorialState tutorialState;

};
