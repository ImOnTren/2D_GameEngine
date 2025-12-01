#include "UI.h"

#include <algorithm>

#include "Engine.h"

std::vector<std::string> UI::DebugMessages = {"Welcome To Click-Craft Creator"};

std::string UI::selectedCategory = "All";
std::string UI::searchFilter = "";
AssetType UI::typeFilter = AssetType::OTHER;
Asset* UI::selectedAsset = nullptr;
int UI::thumbnailSize = 80;
bool UI::showTilesetPreview = false;
char UI::searchBuffer[256] = "";

bool UI::showTileSelectionWindow = false;
Asset* UI::selectedTileset = nullptr;
int UI::selectedTileIndex = -1;
Vector2 UI::selectedTileCoords = {0, 0};
int UI::tilesetTileWidth = 16;
int UI::tilesetTileHeight = 16;

void UI::RenderControlPanel(Engine& engine, Grid& grid) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 4.0f, io.DisplaySize.y - io.DisplaySize.y / 4.0f));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Control panel", NULL);
    RenderModeControls(engine);
    RenderCameraResolutionControls(engine);

    ImGui::End();
}

void UI::RenderDebugConsole() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 4.0f, io.DisplaySize.y / 4.0f));
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - io.DisplaySize.y / 4.0f));
    ImGui::Begin("Debug console", NULL);
    if (ImGui::Button("Clear Console")) {
        ClearDebugMessages();
    }
    ImGui::SameLine();
    ImGui::Text("Messages: %zu", DebugMessages.size());

    ImGui::Separator();

    // Create a child window for scrolling
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);

    // Display all messages (oldest first, newest at bottom)
    for (const auto& message : DebugMessages) {
        ImGui::TextWrapped("%s", message.c_str());
    }

    // Auto-scroll to bottom if new messages were added
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}

void UI::RenderAssetConsole(Engine& engine) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - io.DisplaySize.x / 4.0f, io.DisplaySize.y / 4.0f));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 4.0f, io.DisplaySize.y - io.DisplaySize.y / 4.0f));
    ImGui::Begin("Asset Console", NULL,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    AssetManager& assetManager = engine.GetAssetManager();

    std::vector<std::string> categories = assetManager.GetCategories();
    std::vector<Asset*> allAssets = assetManager.GetAllAssets();

    // Top bar with filters and search
    RenderAssetFilters(assetManager, categories);

    // Main content area
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    ImVec2 leftPanelSize = ImVec2(contentSize.x * 0.2f, contentSize.y);
    ImVec2 rightPanelSize = ImVec2(contentSize.x * 0.8f, contentSize.y);

    // Left panel - Categories
    ImGui::BeginChild("CategoriesPanel", leftPanelSize, true);
    RenderCategoryList(categories, allAssets);
    ImGui::EndChild();

    ImGui::SameLine();

    // Right panel - Asset grid and details
    ImGui::BeginChild("AssetsPanel", rightPanelSize, true);
    RenderAssetGrid(assetManager, allAssets);
    ImGui::EndChild();

    if (showTileSelectionWindow && selectedTileset) {
        RenderTileSelectionWindow(engine, selectedTileset);
    }

    ImGui::End();
}

void UI::RenderAssetFilters(AssetManager& assetManager, std::vector<std::string>& categories) {
    // Search filter
    ImGui::SetNextItemWidth(200);
    char searchBuffer[256] = "";
    if (!searchFilter.empty()) {
        strncpy(searchBuffer, searchFilter.c_str(), sizeof(searchBuffer) - 1);
        searchBuffer[sizeof(searchBuffer) - 1] = '\0';
    }

    if (ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer))) {
        searchFilter = std::string(searchBuffer);
    }
    ImGui::SameLine();

    // Category filter
    ImGui::SetNextItemWidth(150);
    if (ImGui::BeginCombo("Category", selectedCategory.c_str())) {
        if (ImGui::Selectable("All", selectedCategory == "All")) {
            selectedCategory = "All";
        }
        for (const auto& category : categories) {
            if (ImGui::Selectable(category.c_str(), selectedCategory == category)) {
                selectedCategory = category;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();

    // Type filter
    ImGui::SetNextItemWidth(150);
    const char* typeNames[] = {"All", "Texture", "Animated Sprite", "Static Sprite", "Tileset", "Player", "Enemy", "Other"};
    AssetType typeValues[] = {AssetType::OTHER, AssetType::INDIVIDUAL_TEXTURE, AssetType::ANIMATED_SPRITESHEET,
                             AssetType::STATIC_SPRITESHEET, AssetType::TILESET, AssetType::PLAYER, AssetType::ENEMY, AssetType::OTHER};

    int currentTypeIndex = 0;
    for (int i = 0; i < IM_ARRAYSIZE(typeValues); i++) {
        if (typeValues[i] == typeFilter) {
            currentTypeIndex = i;
            break;
        }
    }

    if (ImGui::Combo("Type", &currentTypeIndex, typeNames, IM_ARRAYSIZE(typeNames))) {
        typeFilter = typeValues[currentTypeIndex];
    }
    ImGui::SameLine();

    // Thumbnail size control
    ImGui::SetNextItemWidth(100);
    ImGui::SliderInt("Thumb Size", &thumbnailSize, 40, 120);
    ImGui::SameLine();

    // Asset count
    ImGui::Text("Assets: %d", assetManager.GetAssetCount());
}

void UI::RenderCategoryList(std::vector<std::string>& categories, std::vector<Asset*>& allAssets) {
    ImGui::Text("Categories");
    ImGui::Separator();

    // "All" category
    int allCount = allAssets.size();
    std::string allLabel = "All (" + std::to_string(allCount) + ")";
    if (ImGui::Selectable(allLabel.c_str(), selectedCategory == "All")) {
        selectedCategory = "All";
    }

    // Individual categories
    for (const auto& category : categories) {
        int categoryCount = std::count_if(allAssets.begin(), allAssets.end(),
            [&](Asset* asset) { return asset->category == category; });

        std::string label = category + " (" + std::to_string(categoryCount) + ")";
        if (ImGui::Selectable(label.c_str(), selectedCategory == category)) {
            selectedCategory = category;
        }
    }
}

void UI::RenderAssetGrid(AssetManager& assetManager, std::vector<Asset*>& allAssets) {
    // Filter assets based on current selection
    std::vector<Asset*> filteredAssets;
    for (Asset* asset : allAssets) {
        bool categoryMatch = (selectedCategory == "All") || (asset->category == selectedCategory);
        bool typeMatch = (typeFilter == AssetType::OTHER) || (asset->type == typeFilter);
        bool searchMatch = searchFilter.empty() ||
                          (asset->name.find(searchFilter) != std::string::npos) ||
                          (asset->id.find(searchFilter) != std::string::npos);

        if (categoryMatch && typeMatch && searchMatch) {
            filteredAssets.push_back(asset);
        }
    }

    if (filteredAssets.empty()) {
        ImGui::Text("No assets found");
        return;
    }

    // Calculate grid layout
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int itemsPerRow = std::max(1, (int)(panelWidth / (thumbnailSize + 20)));

    ImGui::Text("Found %zu assets", filteredAssets.size());
    ImGui::Separator();

    // Asset grid
    int itemCount = 0;
    for (Asset* asset : filteredAssets) {
        if (itemCount % itemsPerRow != 0) {
            ImGui::SameLine();
        }

        RenderAssetThumbnail(asset);
        itemCount++;
    }

    // Selected asset details
    if (selectedAsset) {
        ImGui::Separator();
        RenderAssetDetails(selectedAsset, assetManager);
    }
}

void UI::RenderAssetThumbnail(Asset* asset) {
    ImVec2 thumbSize = ImVec2((float)thumbnailSize, (float)thumbnailSize);
    bool isSelected = (selectedAsset == asset);

    ImGui::BeginGroup();

    // Thumbnail background
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (isSelected) {
        drawList->AddRectFilled(cursorPos,
                               ImVec2(cursorPos.x + thumbSize.x + 10, cursorPos.y + thumbSize.y + 25),
                               ImGui::GetColorU32(ImVec4(0.3f, 0.5f, 0.8f, 0.3f)));
    }

    // Thumbnail image
    if (asset->loaded) {
        ImVec2 uv0 = ImVec2(0, 0);
        ImVec2 uv1 = ImVec2(1, 1);
        ImGui::Image((ImTextureID)(uintptr_t)asset->texture.id, thumbSize, uv0, uv1);
    } else {
        ImGui::Dummy(thumbSize);
        ImGui::SameLine();
        ImGui::Text("Not loaded");
    }

    // Asset name
    std::string displayName = asset->name;
    if (displayName.length() > 12) {
        displayName = displayName.substr(0, 12) + "...";
    }

    ImGui::TextWrapped("%s", displayName.c_str());
    ImGui::EndGroup();

    // Selection handling
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
        selectedAsset = asset;
        SetDebugMessage("[ASSET] Selected: " + asset->name);
    }

    // Tooltip on hover
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Name: %s", asset->name.c_str());
        ImGui::Text("ID: %s", asset->id.c_str());
        ImGui::Text("Type: %s", GetAssetTypeName(asset->type).c_str());
        ImGui::Text("Category: %s", asset->category.c_str());
        ImGui::EndTooltip();
    }
}

void UI::RenderAssetDetails(Asset* asset, AssetManager& assetManager) {
    ImGui::Text("Selected Asset Details");
    ImGui::Separator();

    ImGui::Columns(2, "AssetDetails", false);

    // Left column - Large preview
    ImGui::SetColumnWidth(0, 150);
    if (asset->loaded) {
        ImVec2 previewSize = ImVec2(128, 128);
        ImVec2 uv0 = ImVec2(0, 0);
        ImVec2 uv1 = ImVec2(1, 1);
        ImGui::Image((ImTextureID)(uintptr_t)asset->texture.id, previewSize, uv0, uv1);
    }

    ImGui::NextColumn();

    // Right column - Metadata and actions
    ImGui::Text("Name: %s", asset->name.c_str());
    ImGui::Text("ID: %s", asset->id.c_str());
    ImGui::Text("Type: %s", GetAssetTypeName(asset->type).c_str());
    ImGui::Text("Category: %s", asset->category.c_str());
    ImGui::Text("Path: %s", asset->path.c_str());

    if (asset->loaded) {
        ImGui::Text("Dimensions: %dx%d", asset->texture.width, asset->texture.height);

        // Show tileset info if it's a tileset
        if (asset->type == AssetType::TILESET) {
            ImGui::Text("Sub-sprites: %zu", asset->subSprites.size());

            // Calculate grid dimensions
            if (!asset->subSprites.empty()) {
                int cols = asset->texture.width / 16; // Assuming 16x16 tiles
                int rows = asset->texture.height / 16;
                ImGui::Text("Grid: %dx%d tiles", cols, rows);
            }
        }
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "NOT LOADED");
    }

    // Use Asset button
    if (asset->type == AssetType::TILESET) {
        if (ImGui::Button("Select Tile", ImVec2(120, 30))) {
            selectedTileset = asset;
            showTileSelectionWindow = true;
            selectedTileIndex = -1; // Reset selection
            SetDebugMessage("[TILESET] Opening tile selection for: " + asset->name);
        }
    } else {
        if (ImGui::Button("Use Asset", ImVec2(120, 30))) {
            SetDebugMessage("[ASSET] Ready to place: " + asset->name);
            // For non-tileset assets
        }
    }

    // Tileset preview for tileset assets
    if (asset->type == AssetType::TILESET && !asset->subSprites.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Show Tileset", ImVec2(120, 30))) {
            showTilesetPreview = !showTilesetPreview;
        }
    }

    ImGui::Columns(1);

    // Tileset preview window
    if (showTilesetPreview && asset->type == AssetType::TILESET) {
        RenderTilesetPreview(asset);
    }
}

void UI::RenderTilesetPreview(Asset* asset) {
    ImGuiIO io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(600, static_cast<float>(GetScreenHeight())), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x + (io.DisplaySize.x + 600), 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Tileset Preview", &showTilesetPreview, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Tileset: %s (%zu sub-sprites)", asset->name.c_str(), asset->subSprites.size());
    ImGui::Separator();

    // Display all sub-sprites in a grid
    int columns = 12;
    int count = 0;

    for (const auto& spriteRect : asset->subSprites) {
        if (count % columns != 0) {
            ImGui::SameLine();
        }

        // Create a mini texture for each sub-sprite
        ImVec2 uv0 = ImVec2(spriteRect.x / asset->texture.width,
                           (spriteRect.y + spriteRect.height) / asset->texture.height);
        ImVec2 uv1 = ImVec2((spriteRect.x + spriteRect.width) / asset->texture.width,
                           spriteRect.y / asset->texture.height);

        ImGui::Image((ImTextureID)(uintptr_t)asset->texture.id,
                    ImVec2(32, 32), uv0, uv1);

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Sprite %d", count);
            ImGui::Text("Pos: %.0f,%.0f", spriteRect.x, spriteRect.y);
            ImGui::Text("Size: %.0fx%.0f", spriteRect.width, spriteRect.height);
            ImGui::EndTooltip();
        }

        count++;
    }

    ImGui::End();
}

std::string UI::GetAssetTypeName(AssetType type) {
    switch (type) {
        case AssetType::INDIVIDUAL_TEXTURE: return "Texture";
        case AssetType::ANIMATED_SPRITESHEET: return "Animated Sprite";
        case AssetType::STATIC_SPRITESHEET: return "Static Sprite";
        case AssetType::TILESET: return "Tileset";
        case AssetType::PLAYER: return "Player";
        case AssetType::ENEMY: return "Enemy";
        case AssetType::GRASS_BLOCK: return "Grass Block";
        case AssetType::STONE_BLOCK: return "Stone Block";
        case AssetType::TREE: return "Tree";
        case AssetType::HOUSE: return "House";
        case AssetType::CAR: return "Car";
        case AssetType::ANIMAL: return "Animal";
        case AssetType::UI_ELEMENT: return "UI Element";
        case AssetType::FONT: return "Font";
        case AssetType::SOUND: return "Sound";
        case AssetType::MUSIC: return "Music";
        default: return "Other";
    }
}

void UI::RenderTileSelectionWindow(Engine& engine, Asset* tileset) {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(200, 100), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Tile Selection", &showTileSelectionWindow, ImGuiWindowFlags_NoScrollbar)) {
        ImGui::Text("Select a tile from: %s", tileset->name.c_str());
        ImGui::Text("Tileset: %dx%d (%zu tiles)",
                   tileset->texture.width, tileset->texture.height,
                   tileset->subSprites.size());
        ImGui::Separator();

        // Tile size configuration
        ImGui::Text("Tile Size:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::InputInt("Width", &tilesetTileWidth, 1, 10);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::InputInt("Height", &tilesetTileHeight, 1, 10);

        // Clamp tile sizes to reasonable values
        tilesetTileWidth = std::max(1, std::min(tilesetTileWidth, 256));
        tilesetTileHeight = std::max(1, std::min(tilesetTileHeight, 256));

        // Process tileset with current tile size if needed
        if ((tilesetTileWidth != 16 || tilesetTileHeight != 16) &&
            tileset->subSprites.empty()) {
            engine.GetAssetManager().ProcessTileset(tileset, tilesetTileHeight, tilesetTileWidth);
        }

        ImGui::Separator();

        // Display selected tile info
        if (selectedTileIndex >= 0 && selectedTileIndex < tileset->subSprites.size()) {
            Rectangle& selectedRect = tileset->subSprites[selectedTileIndex];
            ImGui::Text("Selected Tile: #%d (%.0f,%.0f)",
                       selectedTileIndex, selectedRect.x, selectedRect.y);

            // Show selected tile preview
            ImVec2 previewSize = ImVec2(64, 64);
            ImVec2 uv0 = ImVec2(selectedRect.x / tileset->texture.width,
                               selectedRect.y / tileset->texture.height);
            ImVec2 uv1 = ImVec2((selectedRect.x + selectedRect.width) / tileset->texture.width,
                               (selectedRect.y + selectedRect.height) / tileset->texture.height);

            ImGui::Image((ImTextureID)(uintptr_t)tileset->texture.id, previewSize, uv0, uv1);

            // Confirm selection button
            ImGui::SameLine();
            if (ImGui::Button("Place This Tile", ImVec2(120, 64))) {
                // Set engine to tile placement mode
                engine.currentTool = Engine::ToolState::PLACING_TILE;
                engine.tileToolState.tileset = tileset;
                engine.tileToolState.selectedTileIndex = selectedTileIndex;
                engine.tileToolState.isPlacingTile = true;
                SetDebugMessage("[TILE] Ready to place tile #" +
                               std::to_string(selectedTileIndex) + " from " + tileset->name);
            }
        } else {
            ImGui::Text("No tile selected - click on a tile below");
        }

        ImGui::Separator();

        // Display the tileset grid for selection
        RenderTilesetGrid(tileset);

    }
    ImGui::End();
}

void UI::RenderTilesetGrid(Asset* tileset) {
    if (!tileset->loaded || tileset->subSprites.empty()) {
        ImGui::Text("Tileset not loaded or no sub-sprites available");
        return;
    }

    ImGui::Text("Click on a tile to select it:");

    // Calculate grid layout
    int textureWidth = tileset->texture.width;
    int textureHeight = tileset->texture.height;
    int tileWidth = tilesetTileWidth;
    int tileHeight = tilesetTileHeight;

    int columns = textureWidth / tileWidth;
    int rows = textureHeight / tileHeight;

    float zoom = 2.0f; // Zoom factor for better visibility
    float displayTileSize = tileWidth * zoom;

    // Calculate how many tiles fit per row in the window
    //float availableWidth = ImGui::GetContentRegionAvail().x;
    //int tilesPerRow = std::max(1, (int)(availableWidth / (displayTileSize + 4)));

    ImGui::BeginChild("TilesetGrid", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);

    int tileIndex = 0;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            if (tileIndex >= tileset->subSprites.size()) break;

            if (x > 0) ImGui::SameLine();

            Rectangle& tileRect = tileset->subSprites[tileIndex];
            bool isSelected = (selectedTileIndex == tileIndex);

            // Draw tile with selection highlight
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            if (isSelected) {
                // Highlight selected tile
                drawList->AddRectFilled(
                    cursorPos,
                    ImVec2(cursorPos.x + displayTileSize + 4, cursorPos.y + displayTileSize + 4),
                    ImGui::GetColorU32(ImVec4(0.2f, 0.8f, 0.2f, 0.3f))
                );
            }

            // Calculate UV coordinates for this tile
            ImVec2 uv0 = ImVec2(tileRect.x / textureWidth, tileRect.y / textureHeight);
            ImVec2 uv1 = ImVec2((tileRect.x + tileRect.width) / textureWidth,
                               (tileRect.y + tileRect.height) / textureHeight);

            // Display the tile
            ImGui::Image((ImTextureID)(uintptr_t)tileset->texture.id,
                        ImVec2(displayTileSize, displayTileSize), uv0, uv1);

            // Tile selection
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                selectedTileIndex = tileIndex;
                selectedTileCoords = {(float)x, (float)y};
                SetDebugMessage("[TILE] Selected tile #" + std::to_string(tileIndex) +
                               " at (" + std::to_string(x) + "," + std::to_string(y) + ")");
            }

            // Tile tooltip
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Tile #%d", tileIndex);
                ImGui::Text("Position: %d,%d", x, y);
                ImGui::Text("Coords: %.0f,%.0f", tileRect.x, tileRect.y);
                ImGui::EndTooltip();
            }

            tileIndex++;
        }

        // Move to next row
        if (y < rows - 1) {
            ImGui::NewLine();
        }
    }

    ImGui::EndChild();
}

void UI::RenderPlayModeWindow(Engine& engine) {
    if (!engine.playModeWindowOpen) return;

    //auto& resolutions = engine.GetAvailableResolutions();
    //int currentIndex = engine.GetSelectedResolutionIndex();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - io.DisplaySize.x / 4.0f, io.DisplaySize.y - io.DisplaySize.y / 4.0f));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 4.0f, 0));
    //std::cout << io.DisplaySize.x << " " << io.DisplaySize.y << std::endl;

    if (ImGui::Begin("Play Mode", &engine.playModeWindowOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        std::string fps = "FPS: " + std::to_string(io.Framerate);
        const char* fpsText = fps.c_str();
        ImGui::Text(fpsText);
        // Calculate UV coordinates to fix inversion
        ImVec2 uv0 = ImVec2(0, 1);
        ImVec2 uv1 = ImVec2(1, 0);

        // Maintain aspect ratio
        float texW = (float)engine.playModeTexture.texture.width;
        float texH = (float)engine.playModeTexture.texture.height;

        // Compute the maximum integer scale that fits in the window
        float scaleX = std::floor(contentSize.x / texW);
        float scaleY = std::floor(contentSize.y / texH);
        float scale = std::max(1.0f, std::min(scaleX, scaleY));

        ImVec2 displaySize;
        displaySize.x = texW * scale;
        displaySize.y = texH * scale;

        // Center the image
        ImVec2 pos = ImGui::GetCursorPos();
        pos.x += (contentSize.x - displaySize.x) * 0.5f;
        pos.y += (contentSize.y - displaySize.y) * 0.5f;

        // Snap to integer pixels
        pos.x = std::floor(pos.x);
        pos.y = std::floor(pos.y);
        displaySize.x = std::floor(displaySize.x);
        displaySize.y = std::floor(displaySize.y);

        ImGui::SetCursorPos(pos);

        // Display the texture
        ImGui::Image(
            (ImTextureID)(uintptr_t)engine.playModeTexture.texture.id,
            displaySize,
            uv0,
            uv1,
            ImVec4(1, 1, 1, 1),
            ImVec4(0, 0, 0, 0)
        );
    }
    ImGui::End();
}

void UI::RenderModeControls(Engine& engine) {
    ImGui::Separator();
    ImGui::Text("Engine Mode:");

    if (ImGui::RadioButton("Edit Mode", engine.currentMode == Engine::Mode::EDIT)) {
        engine.StopPlayMode();
        engine.ResetTool();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Play Mode", engine.currentMode == Engine::Mode::PLAY)) {
        engine.StartPlayMode();
        engine.ResetTool();
    }

    if (engine.currentMode == Engine::Mode::EDIT) {
        ImGui::Separator();
        ImGui::Text("Edit Tools:");

        // Player placement/removal
        if (ImGui::Button("Place Player", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::PLACING_PLAYER) ?
                Engine::ToolState::NONE : Engine::ToolState::PLACING_PLAYER;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Player", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_PLAYER) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_PLAYER;
        }

        // Enemy placement/removal
        if (ImGui::Button("Place Enemy", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::PLACING_ENEMY) ?
                Engine::ToolState::NONE : Engine::ToolState::PLACING_ENEMY;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Enemy", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_ENEMY) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_ENEMY;
        }

        if (ImGui::Button("Place Tile", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::PLACING_TILE) ?
                Engine::ToolState::NONE : Engine::ToolState::PLACING_TILE;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Tile", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_TILE) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_TILE;
        }

        // Show current tool status
        ImGui::Text("Current Tool: ");
        ImGui::SameLine();
        switch (engine.currentTool) {
            case Engine::ToolState::NONE: ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "None"); break;
            case Engine::ToolState::PLACING_PLAYER: ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Placing Player"); break;
            case Engine::ToolState::REMOVING_PLAYER: ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Removing Player"); break;
            case Engine::ToolState::PLACING_ENEMY: ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Placing Enemy"); break;
            case Engine::ToolState::REMOVING_ENEMY: ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Removing Enemy"); break;
            case Engine::ToolState::PLACING_TILE: ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Placing Tile"); break;
            case Engine::ToolState::REMOVING_TILE: ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Removing Tile"); break;
        }

        // Show enemy count
        ImGui::Text("Enemies: %d/10", engine.GetEnemyCount());

        if (ImGui::Button("Clear Tool", ImVec2(120, 0))) {
            engine.ResetTool();
        }
    }
}

void UI::RenderCameraResolutionControls(Engine& engine) {
    ImGui::Separator();
    ImGui::Text("Player Camera Settings:");

    // Use public method to check if player exists
    if (!engine.HasPlayer()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Place a player to see camera preview");
    } else {
        ImGui::Text("Camera preview visible around player");
    }

    // Resolution dropdown using public methods
    ImGui::Text("Resolution:");

    // Get resolutions via public method
    const auto& resolutions = engine.GetAvailableResolutions();
    int currentIndex = engine.GetSelectedResolutionIndex();

    // Create combo box labels
    std::vector<const char*> resolutionLabels;
    for (const auto& res : resolutions) {
        resolutionLabels.push_back(res.name);
    }

    if (ImGui::Combo("##Resolution", &currentIndex, resolutionLabels.data(), resolutionLabels.size())) {
        // Use public setter method
        engine.SetSelectedResolutionIndex(currentIndex);
    }

    // Display current resolution info
    if (currentIndex >= 0 && currentIndex < resolutions.size()) {
        auto& currentRes = resolutions[currentIndex];
        ImGui::Text("Current: %dx%d (%.1f:1)",
                   currentRes.width, currentRes.height,
                   (float)currentRes.width / currentRes.height);

        // Aspect ratio info
        float aspect = (float)currentRes.width / currentRes.height;
        const char* aspectName = "Custom";
        if (abs(aspect - 16.0f/9.0f) < 0.01f) aspectName = "16:9";
        else if (abs(aspect - 4.0f/3.0f) < 0.01f) aspectName = "4:3";
        else if (abs(aspect - 1.0f) < 0.01f) aspectName = "1:1";

        ImGui::Text("Aspect Ratio: %s", aspectName);
    }
}
