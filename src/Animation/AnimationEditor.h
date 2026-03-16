// AnimationEditor.h
#pragma once

#include <string>
#include <vector>
#include "raylib.h"
#include "Animation/Animation.h"
#include "AssetManagement/AssetManager.h"


class Engine;
class AssetManager;

class AnimationEditor {
public:
    AnimationEditor() = default;
    ~AnimationEditor();

    // Main render function - call this from your UI rendering
    void Render(Engine& engine);

    // Check if the editor window is open
    bool IsOpen() const { return windowOpen; }
    void SetOpen(bool open) { windowOpen = open; }

    void OpenForAsset(Asset* asset);  // Open editor for specific asset
    void ClearTargetAsset() { targetAsset = nullptr; }

private:
    bool windowOpen = false;

    // Current sprite sheet being edited
    std::string currentTexturePath;
    Texture2D currentTexture = {0};
    bool textureLoaded = false;

    // Frame settings
    int frameWidth = 32;
    int frameHeight = 32;

    // Detected grid info
    int columnsInSheet = 0;
    int rowsInSheet = 0;

    Asset* targetAsset = nullptr;

    // Animation definition in progress
    struct AnimationDefinition {
        std::string name;
        int row = 0;
        int frameCount = 4;
        float frameRate = 8.0f;
        bool loop = true;
        AnimationTrigger trigger = AnimationTrigger::LOOP;
        AnimationDirection direction = AnimationDirection::NONE;
        bool isAutoFlipped = false;  // True if this was auto-created as LEFT from RIGHT
    };

    std::vector<AnimationDefinition> definedAnimations;

    // UI state
    char nameBuffer[64] = "idle";
    int selectedRow = 0;
    int selectedFrameCount = 4;
    float selectedFrameRate = 8.0f;
    bool selectedLoop = true;
    int selectedTrigger = 0;  // Index into trigger options
    int selectedDirection = 0; // Index into direction options
    bool autoCreateLeft = true; // Auto-create LEFT when adding RIGHT

    // Preview state
    int previewAnimIndex = -1;
    int previewFrame = 0;
    float previewTimer = 0.0f;
    bool previewFlipped = false;

    // File browser state
    struct FileEntry {
        std::string fullPath;
        std::string displayPath;  // Relative path for display
    };
    std::vector<FileEntry> foundPngFiles;
    int selectedFileIndex = -1;
    char searchPathBuffer[256] = "../src/assets";
    std::string currentSearchPath;
    bool needsRescan = false;

    // Helper functions
    void RenderFileSelector();
    void RenderFrameSettings();
    void RenderSpriteSheetPreview();
    void RenderAnimationDefiner();
    void RenderDefinedAnimationsList();
    void RenderAnimationPreview();
    void RenderSaveSection(Engine& engine);

    void LoadTexture(const std::string& path);
    void UnloadCurrentTexture();
    void ScanForPngFiles(const std::string& directory);
    void UpdatePreview(float deltaTime);
    void RecalculateGridSize();
    void SaveAnimationsToAsset();

    // Helper to get direction suffix
    std::string GetDirectionSuffix(AnimationDirection dir) const;
};