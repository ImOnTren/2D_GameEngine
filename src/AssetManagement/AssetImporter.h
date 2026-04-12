#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "raylib.h"
#include "AssetManager.h"

class Engine;

class AssetImporter {
public:
    AssetImporter() = default;
    ~AssetImporter();

    // Main render function - call from UI
    void Render(Engine& engine);

    // Check if the importer window is open
    bool IsOpen() const { return windowOpen; }
    void SetOpen(bool open) { windowOpen = open; }

private:
    bool windowOpen = false;

    // File browser state
    struct FileEntry {
        std::string fullPath;
        std::string displayPath;  // Relative path for display
        std::string filename;
    };
    std::vector<FileEntry> foundPngFiles;
    int selectedFileIndex = -1;
    char searchPathBuffer[256] = "assets";
    bool needsRescan = true;

    // Currently loaded texture for preview
    std::string currentTexturePath;
    Texture2D currentTexture = {0};
    bool textureLoaded = false;

    // Frame selection (for selecting a specific frame from spritesheet)
    int frameWidth = 32;
    int frameHeight = 32;
    int selectedFrameX = 0;  // Grid position
    int selectedFrameY = 0;  // Grid position
    bool frameSelected = false;

    // Grid info
    int columnsInSheet = 0;
    int rowsInSheet = 0;

    // Asset creation parameters
    char assetIdBuffer[128] = "";
    char assetNameBuffer[128] = "";
    char assetCategoryBuffer[128] = "Uncategorized";
    int selectedAssetType = 0;  // Index into asset type array

    // Asset type options
    struct AssetTypeOption {
        const char* key;
        AssetType type;
    };
    std::vector<AssetTypeOption> assetTypes = {
        {"importer.asset_type.individual_texture", AssetType::INDIVIDUAL_TEXTURE},
        {"importer.asset_type.static_spritesheet", AssetType::STATIC_SPRITESHEET},
        {"importer.asset_type.animated_spritesheet", AssetType::ANIMATED_SPRITESHEET},
        {"importer.asset_type.tileset", AssetType::TILESET},
        {"importer.asset_type.player", AssetType::PLAYER},
        {"importer.asset_type.enemy", AssetType::ENEMY}
    };

    // Tileset specific
    bool isTileset = false;
    int tilesetTileWidth = 16;
    int tilesetTileHeight = 16;

    // UI helper functions
    void RenderFileBrowser();
    void RenderTexturePreview();
    void RenderFrameSelector();
    void RenderAssetParameters();
    void RenderCreateButton(Engine& engine);

    // Utility functions
    void ScanForPngFiles(const std::string& directory);
    void LoadTexture(const std::string& path);
    void UnloadCurrentTexture();
    void RecalculateGridSize();
    void CreateAsset(Engine& engine);
    void ResetForm();

    // Helper to generate unique ID
    std::string GenerateUniqueId(const std::string& baseName, AssetManager& assetManager);
};
