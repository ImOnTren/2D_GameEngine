#include "UI.h"
#include <algorithm>
#include "Engine.h"
#include "../Managers/LocalizitionManager.h"
#define TR(key) Localization::Get(key)

std::vector<std::string> UI::DebugMessages = {"Welcome To Click-Craft Creator"};

std::string UI::selectedCategory = "All";
std::string UI::searchFilter;
AssetType UI::typeFilter = AssetType::OTHER;
Asset* UI::selectedAsset = nullptr;
int UI::thumbnailSize = 85;
bool UI::showTilesetPreview = false;
char UI::searchBuffer[256] = "";
Entity* UI::selectedSceneEntity = nullptr;

bool UI::showTileSelectionWindow = false;
Asset* UI::selectedTileset = nullptr;
int UI::selectedTileIndex = -1;
Vector2 UI::selectedTileCoords = {0, 0};
int UI::tilesetTileWidth = 16;
int UI::tilesetTileHeight = 16;
bool UI::tileMapModified = false;
bool UI::bulkEditActive = false;
int UI::bulkEditStartX = 0;
int UI::bulkEditStartY = 0;
int UI::bulkEditEndX = 0;
int UI::bulkEditEndY = 0;
float UI::controlPanelWidthRatio = 0.25f;
float UI::assetConsoleHeightRatio = 0.25f;

void UI::RenderControlPanel(Engine& engine, const Grid& grid) {
    ImGuiIO& io = ImGui::GetIO();
    float panelWidth = io.DisplaySize.x * controlPanelWidthRatio;
    float fixedDebugHeight = io.DisplaySize.y / 4.0f;

    ImGui::SetNextWindowSize(ImVec2(panelWidth, io.DisplaySize.y - fixedDebugHeight));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("Control Panel##MainWindow", nullptr);

    RenderModeControls(engine);
    RenderCameraResolutionControls(engine);
    ImGui::Separator();
    ImGui::Text("%s", TR("asset_management"));


    if (ImGui::Button(TR("import_asset"), ImVec2(-1, 35))) {
        engine.GetAssetImporter().SetOpen(true);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Open Asset Importer to load textures dynamically");
    }

    // Keyboard shortcut
    if (IsKeyPressed(KEY_F8)) {
        engine.GetAssetImporter().SetOpen(!engine.GetAssetImporter().IsOpen());
    }

    RenderTileSceneContext(engine, grid);

    ImGui::End();
}

void UI::RenderDebugConsole() {
    ImGuiIO& io = ImGui::GetIO();
    float panelWidth = io.DisplaySize.x * controlPanelWidthRatio;
    float fixedDebugHeight = io.DisplaySize.y / 4.0f;

    ImGui::SetNextWindowSize(ImVec2(panelWidth, fixedDebugHeight));
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - fixedDebugHeight));
    ImGui::Begin("Debug Console##DebugWindow", nullptr);
    if (ImGui::Button(TR("clear_console"))) {
        ClearDebugMessages();
    }
    ImGui::SameLine();
    ImGui::Text("Messages: %zu", DebugMessages.size());

    ImGui::Separator();

    // Create a child window for scrolling
    ImGui::BeginChild("ScrollingRegion##DebugConsole", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);

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
    float panelWidth = io.DisplaySize.x * controlPanelWidthRatio;
    float consoleHeight = io.DisplaySize.y * assetConsoleHeightRatio;

    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - panelWidth, consoleHeight));
    ImGui::SetNextWindowPos(ImVec2(panelWidth, io.DisplaySize.y - consoleHeight));
    ImGui::Begin("Asset Console", nullptr,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    AssetManager& assetManager = engine.GetAssetManager();

    if (ImGui::Button("+ Import New Asset", ImVec2(150, 30))) {
        engine.GetAssetImporter().SetOpen(true);
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Add textures without hardcoding");
    ImGui::Separator();

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
    RenderAssetGrid(engine, assetManager, allAssets);
    ImGui::EndChild();

    if (showTileSelectionWindow && selectedTileset) {
        RenderTileSelectionWindow(engine, selectedTileset);
    }

    ImGui::End();
}

void UI::RenderAssetFilters(AssetManager& assetManager, const std::vector<std::string>& categories) {
    // Search filter
    ImGui::SetNextItemWidth(200);
    char searchBarBuffer[256] = "";
    if (!searchFilter.empty()) {
        strncpy(searchBarBuffer, searchFilter.c_str(), sizeof(searchBarBuffer) - 1);
        searchBarBuffer[sizeof(searchBarBuffer) - 1] = '\0';
    }

    if (ImGui::InputText(TR("search"), searchBarBuffer, sizeof(searchBarBuffer))) {
        searchFilter = std::string(searchBarBuffer);
    }
    ImGui::SameLine();

    // Category filter
    ImGui::SetNextItemWidth(150);
    if (ImGui::BeginCombo(TR("category"), selectedCategory.c_str())) {
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
    ImGui::SliderInt("Thumb Size", &thumbnailSize, 85, 120);
    ImGui::SameLine();

    // Asset count
    ImGui::Text("Assets: %d", assetManager.GetAssetCount());
    ImGui::SameLine();

    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("Console Size", &UI::assetConsoleHeightRatio, 0.15f, 0.60f, "%.2f");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Adjust Asset Console Height");
}

void UI::RenderCategoryList(const std::vector<std::string>& categories, std::vector<Asset*>& allAssets) {
    ImGui::Text("Categories");
    ImGui::Separator();

    // "All" category
    const int allCount = static_cast<int>(allAssets.size());
    std::string allLabel = "All (" + std::to_string(allCount) + ")";
    if (ImGui::Selectable(allLabel.c_str(), selectedCategory == "All")) {
        selectedCategory = "All";
    }

    // Individual categories
    for (const auto& category : categories) {
        int categoryCount = static_cast<int>(std::count_if(allAssets.begin(), allAssets.end(),
            [&](const Asset* asset) { return asset->category == category; }));

        std::string label = category + " (" + std::to_string(categoryCount) + ")";
        if (ImGui::Selectable(label.c_str(), selectedCategory == category)) {
            selectedCategory = category;
        }
    }
}

void UI::RenderAssetGrid(Engine& engine, AssetManager& assetManager, const std::vector<Asset*>& allAssets) {
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
    const float panelWidth = ImGui::GetContentRegionAvail().x;
    const int itemsPerRow = std::max(1, static_cast<int>(panelWidth) / (thumbnailSize + 20));

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
        RenderAssetDetails(engine, selectedAsset, assetManager);
    }
}

void UI::RenderAssetThumbnail(Asset* asset) {
    ImVec2 thumbSize = ImVec2(static_cast<float>(thumbnailSize), static_cast<float>(thumbnailSize));
    bool isSelected = (selectedAsset == asset);

    ImGui::BeginGroup();

    // Get position before anything is drawn
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw background for selected items
    if (isSelected) {
        drawList->AddRectFilled(cursorPos,
                               ImVec2(cursorPos.x + thumbSize.x + 10, cursorPos.y + thumbSize.y + 25),
                               ImGui::GetColorU32(ImVec4(0.3f, 0.5f, 0.8f, 0.3f)));
    }

    // FIX: Use InvisibleButton to reserve space and handle clicks
    // This creates a proper bounding box that ImGui respects
    ImGui::PushID(asset->id.c_str());
    ImGui::InvisibleButton("thumb", thumbSize);
    ImGui::PopID();

    // Get the exact bounds of the button
    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageMax = ImGui::GetItemRectMax();

    // Check if item was clicked
    bool clicked = ImGui::IsItemClicked();
    bool hovered = ImGui::IsItemHovered();

    // Now draw the actual image using the draw list (this respects clipping)
    if (asset->loaded) {
        ImVec2 uv0, uv1;

        // Check if this asset has a selected frame (SpriteSourceRect is set)
        if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
            // Use the selected frame from the spritesheet
            uv0 = ImVec2(
                asset->SpriteSourceRect.x / static_cast<float>(asset->texture.width),
                asset->SpriteSourceRect.y / static_cast<float>(asset->texture.height)
            );
            uv1 = ImVec2(
                (asset->SpriteSourceRect.x + asset->SpriteSourceRect.width) / static_cast<float>(asset->texture.width),
                (asset->SpriteSourceRect.y + asset->SpriteSourceRect.height) / static_cast<float>(asset->texture.height)
            );
        } else {
            // Use full texture
            uv0 = ImVec2(0, 0);
            uv1 = ImVec2(1, 1);
        }

        // Calculate proper aspect-correct size that fits
        float texWidth = asset->SpriteSourceRect.width > 0 ?
                        asset->SpriteSourceRect.width :
                        static_cast<float>(asset->texture.width);
        float texHeight = asset->SpriteSourceRect.height > 0 ?
                         asset->SpriteSourceRect.height :
                         static_cast<float>(asset->texture.height);

        float scaleX = thumbSize.x / texWidth;
        float scaleY = thumbSize.y / texHeight;
        float scale = std::min(scaleX, scaleY);

        ImVec2 displaySize(texWidth * scale, texHeight * scale);

        // Center within the button bounds
        ImVec2 imageCenter = ImVec2(
            imageMin.x + (thumbSize.x - displaySize.x) * 0.5f,
            imageMin.y + (thumbSize.y - displaySize.y) * 0.5f
        );

        ImVec2 imageFinalMin = imageCenter;
        ImVec2 imageFinalMax = ImVec2(imageCenter.x + displaySize.x, imageCenter.y + displaySize.y);

        // Draw using AddImage (which properly clips to parent bounds)
        drawList->AddImage(
            (ImTextureID)static_cast<uintptr_t>(asset->texture.id),
            imageFinalMin,
            imageFinalMax,
            uv0,
            uv1,
            IM_COL32(255, 255, 255, 255)
        );
    } else {
        // Draw "Not loaded" text centered
        const char* notLoadedText = "Not loaded";
        ImVec2 textSize = ImGui::CalcTextSize(notLoadedText);
        ImVec2 textPos = ImVec2(
            imageMin.x + (thumbSize.x - textSize.x) * 0.5f,
            imageMin.y + (thumbSize.y - textSize.y) * 0.5f
        );
        drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), notLoadedText);
    }

    // Asset name below thumbnail
    std::string displayName = asset->name;
    if (displayName.length() > 12) {
        displayName = displayName.substr(0, 12) + "...";
    }

    ImGui::TextWrapped("%s", displayName.c_str());
    ImGui::EndGroup();

    // Selection handling
    if (clicked) {
        selectedAsset = asset;
        SetDebugMessage("[ASSET] Selected: " + asset->name);
    }

    // Tooltip on hover
    if (hovered) {
        ImGui::BeginTooltip();
        ImGui::Text("Name: %s", asset->name.c_str());
        ImGui::Text("ID: %s", asset->id.c_str());
        ImGui::Text("Type: %s", GetAssetTypeName(asset->type).c_str());
        ImGui::Text("Category: %s", asset->category.c_str());

        // Show frame info if it's a selected frame
        if (asset->SpriteSourceRect.width > 0) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                             "Frame: %.0fx%.0f at (%.0f, %.0f)",
                             asset->SpriteSourceRect.width,
                             asset->SpriteSourceRect.height,
                             asset->SpriteSourceRect.x,
                             asset->SpriteSourceRect.y);
        }

        ImGui::EndTooltip();
    }
}

bool UI::IsMouseOverUI() {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

void UI::RenderAssetDetails(Engine& engine, Asset* asset, AssetManager& assetManager) {
    ImGui::Text("Selected Asset Details");
    ImGui::Separator();

    ImGui::Columns(2, "AssetDetails", false);

    // Left column - Large preview
    ImGui::SetColumnWidth(0, 150);
    if (asset->loaded) {
        ImVec2 previewSize = ImVec2(128, 128);
        ImVec2 uv0, uv1;

        // Check if this asset has a selected frame
        if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
            // Show the selected frame
            uv0 = ImVec2(
                asset->SpriteSourceRect.x / static_cast<float>(asset->texture.width),
                asset->SpriteSourceRect.y / static_cast<float>(asset->texture.height)
            );
            uv1 = ImVec2(
                (asset->SpriteSourceRect.x + asset->SpriteSourceRect.width) / static_cast<float>(asset->texture.width),
                (asset->SpriteSourceRect.y + asset->SpriteSourceRect.height) / static_cast<float>(asset->texture.height)
            );
        } else {
            // Show full texture
            uv0 = ImVec2(0, 0);
            uv1 = ImVec2(1, 1);
        }

        ImGui::Image((ImTextureID)static_cast<uintptr_t>(asset->texture.id), previewSize, uv0, uv1);

        // Add indicator if showing selected frame
        if (asset->SpriteSourceRect.width > 0) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Selected Frame");
        }
    }

    ImGui::NextColumn();

    // Right column - Metadata and actions
    ImGui::Text("Name: %s", asset->name.c_str());
    ImGui::Text("ID: %s", asset->id.c_str());
    ImGui::Text("Type: %s", GetAssetTypeName(asset->type).c_str());
    ImGui::Text("Category: %s", asset->category.c_str());
    ImGui::Text("Path: %s", asset->path.c_str());

    if (asset->loaded) {
        ImGui::Text("Texture Size: %dx%d", asset->texture.width, asset->texture.height);

        // Show frame info if available
        if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Selected Frame:");
            ImGui::Text("Size: %.0fx%.0f",
                       asset->SpriteSourceRect.width,
                       asset->SpriteSourceRect.height);
            ImGui::Text("Position: (%.0f, %.0f)",
                       asset->SpriteSourceRect.x,
                       asset->SpriteSourceRect.y);

            // Calculate frame coordinates
            if (asset->SpriteSize.x > 0 && asset->SpriteSize.y > 0) {
                int frameX = static_cast<int>(asset->SpriteSourceRect.x / asset->SpriteSize.x);
                int frameY = static_cast<int>(asset->SpriteSourceRect.y / asset->SpriteSize.y);
                ImGui::Text("Frame: Column %d, Row %d", frameX, frameY);
            }
        }

        // Show tileset info if it's a tileset
        if (asset->type == AssetType::TILESET) {
            ImGui::Separator();
            ImGui::Text("Tileset Tiles: %zu", asset->subSprites.size());

            if (!asset->subSprites.empty()) {
                int cols = asset->texture.width / static_cast<int>(asset->SpriteSize.x);
                int rows = asset->texture.height / static_cast<int>(asset->SpriteSize.y);
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
            if (asset->type == AssetType::PLAYER) {
                engine.currentTool = Engine::ToolState::PLACING_PLAYER;
                engine.GetPlayerManager().SetPlacementAsset(asset);
                SetDebugMessage("[PLAYER] Ready to place player using asset: " + asset->name);
            } else if (asset->type == AssetType::ENEMY) {
                engine.currentTool = Engine::ToolState::PLACING_ENEMY;
                engine.GetEnemyManager().SetPlacementAsset(asset);
                SetDebugMessage("[ENEMY] Ready to place enemy using asset: " + asset->name);
            } else {
                engine.currentTool = Engine::ToolState::PLACING_ASSET;
                engine.assetToolState.asset = asset;
                engine.assetToolState.isPlacing = true;
                SetDebugMessage("[ASSET] Ready to place: " + asset->name);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Asset", ImVec2(120, 30))) {
            engine.currentTool = Engine::ToolState::REMOVING_ASSET;
            SetDebugMessage("[ASSET] Click on an asset to remove it");
        }
    }

    if (asset->type == AssetType::ANIMATED_SPRITESHEET ||
        asset->type == AssetType::STATIC_SPRITESHEET ||
        asset->type == AssetType::INDIVIDUAL_TEXTURE ||
        asset->type == AssetType::PLAYER ||
        asset->type == AssetType::ENEMY) {

        if (asset->loaded && asset->texture.id != 0) {
            ImGui::SameLine();

            // Color the button differently if asset already has animations
            if (asset->HasAnimations()) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            }

            if (ImGui::Button("Edit Animations", ImVec2(120, 30))) {
                engine.GetAnimationEditor().OpenForAsset(asset);
                SetDebugMessage("[ANIMATION] Opened animation editor for: " + asset->name);
            }

            if (asset->HasAnimations()) {
                ImGui::PopStyleColor();
            }

            // Show animation count if has animations
            if (asset->HasAnimations() && asset->animationSet) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                                 "(%zu anims)",
                                 asset->animationSet->animations.size());
            }
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

    if (asset->type != AssetType::TILESET) {if (asset->type != AssetType::TILESET) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Placed Asset Settings:");

        Scene* scene = engine.GetCurrentScene();
        if (scene) {
            auto& entities = scene->GetEditModeEntities();
            std::vector<Entity*> matchingEntities;

            for (auto& entity : entities) {
                if (auto* staticEntity = dynamic_cast<StaticEntity*>(entity.get())) {
                    if (staticEntity->GetAsset() == asset) {
                        matchingEntities.push_back(staticEntity);
                    }
                } else if (auto* playerEntity = dynamic_cast<PlayerEntity*>(entity.get())) {
                    if (playerEntity->GetAsset() == asset) {
                        matchingEntities.push_back(playerEntity);
                    }
                } else if (auto* enemyEntity = dynamic_cast<EnemyEntity*>(entity.get())) {
                    if (enemyEntity->GetAsset() == asset) {
                        matchingEntities.push_back(enemyEntity);
                    }
                }
            }

            if (matchingEntities.empty()) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "No instances placed yet");
            } else {
                ImGui::Text("Placed instances: %d", (int)matchingEntities.size());

                ImGui::Spacing();
                if (ImGui::Button("Select in Scene", ImVec2(-1, 30))) {
                    engine.currentTool = Engine::ToolState::NONE;
                    selectedSceneEntity = nullptr;
                    SetDebugMessage("[ASSET] Click an instance in the scene to edit it");
                }

                if (engine.currentMode == Engine::Mode::EDIT &&
                    !IsMouseOverUI() &&
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

                    Vector2 mouseScreen = GetMousePosition();
                    Vector2 worldPos = GetScreenToWorld2D(mouseScreen, engine.GetGrid().GetCamera());

                    for (auto* entity : matchingEntities) {
                        Rectangle bounds = entity->GetDrawBounds();
                        if (CheckCollisionPointRec(worldPos, bounds)) {
                            selectedSceneEntity = entity;
                            break;
                        }
                    }
                    }

                if (selectedSceneEntity && std::find(matchingEntities.begin(), matchingEntities.end(), selectedSceneEntity) == matchingEntities.end()) {
                    selectedSceneEntity = nullptr;
                }

                if (selectedSceneEntity &&
                    selectedSceneEntity->SupportsScaling() &&
                    std::find(matchingEntities.begin(), matchingEntities.end(), selectedSceneEntity) != matchingEntities.end()) {

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1), "Selected Instance:");

                    if (auto* staticEntity = dynamic_cast<StaticEntity*>(selectedSceneEntity)) {
                        ImGui::Text("Type: Static");
                        ImGui::Text("Position: (%d, %d)", staticEntity->GetGridX(), staticEntity->GetGridY());
                        ImGui::Text("Layer: %d", staticEntity->GetLayer());
                    } else if (auto* playerEntity = dynamic_cast<PlayerEntity*>(selectedSceneEntity)) {
                        ImGui::Text("Type: Player");
                        ImGui::Text("Position: (%d, %d)", playerEntity->GetGridX(), playerEntity->GetGridY());
                        ImGui::Text("Layer: %d", playerEntity->GetLayer());
                    } else if (auto* enemyEntity = dynamic_cast<EnemyEntity*>(selectedSceneEntity)) {
                        ImGui::Text("Type: Enemy");
                        ImGui::Text("Position: (%d, %d)", enemyEntity->GetGridX(), enemyEntity->GetGridY());
                        ImGui::Text("Layer: %d", enemyEntity->GetLayer());
                    }

                    float currentScale = selectedSceneEntity->GetScale();
                    ImGui::Text("Current Scale: %.2fx", currentScale);

                    ImGui::Spacing();
                    ImGui::Text("Adjust Scale:");

                    float scaleValue = currentScale;
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::SliderFloat("##ScaleSlider", &scaleValue, 0.25f, 4.0f, "%.2fx")) {
                        selectedSceneEntity->SetScale(scaleValue);
                        SetDebugMessage("[ASSET] Scale set to " + std::to_string(scaleValue));
                    }

                    ImGui::Text("Quick Scale:");
                    if (ImGui::Button("0.25x", ImVec2(60, 25))) selectedSceneEntity->SetScale(0.25f);
                    ImGui::SameLine();
                    if (ImGui::Button("0.5x", ImVec2(60, 25))) selectedSceneEntity->SetScale(0.5f);
                    ImGui::SameLine();
                    if (ImGui::Button("0.75x", ImVec2(60, 25))) selectedSceneEntity->SetScale(0.75f);
                    ImGui::SameLine();
                    if (ImGui::Button("1x", ImVec2(60, 25))) selectedSceneEntity->SetScale(1.0f);

                    if (ImGui::Button("1.5x", ImVec2(60, 25))) selectedSceneEntity->SetScale(1.5f);
                    ImGui::SameLine();
                    if (ImGui::Button("2x", ImVec2(60, 25))) selectedSceneEntity->SetScale(2.0f);
                    ImGui::SameLine();
                    if (ImGui::Button("3x", ImVec2(60, 25))) selectedSceneEntity->SetScale(3.0f);
                    ImGui::SameLine();
                    if (ImGui::Button("4x", ImVec2(60, 25))) selectedSceneEntity->SetScale(4.0f);

                    ImGui::Spacing();
                    ImGui::Separator();
                    if (matchingEntities.size() > 1) {
                        ImGui::Text("Apply to All Instances:");
                        if (ImGui::Button("Scale All to Match", ImVec2(-1, 30))) {
                            float scaleToApply = selectedSceneEntity->GetScale();
                            for (auto* entity : matchingEntities) {
                                if (entity->SupportsScaling()) {
                                    entity->SetScale(scaleToApply);
                                }
                            }
                        }
                    }

                    ImGui::Spacing();
                    if (ImGui::Button("Deselect", ImVec2(-1, 25))) {
                        selectedSceneEntity = nullptr;
                        SetDebugMessage("[ASSET] Deselected asset instance");
                    }

                    } else if (!matchingEntities.empty()) {
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(1, 1, 0, 1),
                            "Click 'Select in Scene',\nthen click an instance\nto adjust its scale");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Text("All Instances:");
                        ImGui::BeginChild("InstancesList", ImVec2(0, 100), true);

                        for (size_t i = 0; i < matchingEntities.size(); i++) {
                            auto* entity = matchingEntities[i];
                            ImGui::PushID(static_cast<int>(i));

                            std::string typeName = "Entity";
                            int gx = 0;
                            int gy = 0;
                            int entLayer = 0;

                            if (auto* staticEntity = dynamic_cast<StaticEntity*>(entity)) {
                                typeName = "Static";
                                gx = staticEntity->GetGridX();
                                gy = staticEntity->GetGridY();
                                entLayer = staticEntity->GetLayer();
                            } else if (auto* playerEntity = dynamic_cast<PlayerEntity*>(entity)) {
                                typeName = "Player";
                                gx = playerEntity->GetGridX();
                                gy = playerEntity->GetGridY();
                                entLayer = playerEntity->GetLayer();
                            } else if (auto* enemyEntity = dynamic_cast<EnemyEntity*>(entity)) {
                                typeName = "Enemy";
                                gx = enemyEntity->GetGridX();
                                gy = enemyEntity->GetGridY();
                                entLayer = enemyEntity->GetLayer();
                            }

                            char label[128];
                            snprintf(label, sizeof(label), "%s Pos:(%d,%d) Scale:%.2fx Layer:%d",
                                     typeName.c_str(), gx, gy, entity->GetScale(), entLayer);

                            if (ImGui::Selectable(label, selectedSceneEntity == entity)) {
                                selectedSceneEntity = entity;
                            }

                            ImGui::PopID();
                        }

                        ImGui::EndChild();
                    }
                }
            }
        }

        // Tileset preview window
        if (showTilesetPreview && asset->type == AssetType::TILESET) {
            RenderTilesetPreview(asset);
        }
    }
}

void UI::RenderTilesetPreview(const Asset* asset) {
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
        ImVec2 uv0 = ImVec2(spriteRect.x / static_cast<float>(asset->texture.width),
                           (spriteRect.y + spriteRect.height) / static_cast<float>(asset->texture.height));
        ImVec2 uv1 = ImVec2((spriteRect.x + spriteRect.width) / static_cast<float>(asset->texture.width),
                           spriteRect.y / static_cast<float>(asset->texture.height));

        ImGui::Image((ImTextureID)static_cast<uintptr_t>(asset->texture.id),
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

void UI::RenderTileSceneContext(Engine& engine, const Grid& grid) {
    ImGuiIO io = ImGui::GetIO();

    static int contextTileX = 0;
    static int contextTileY = 0;

    if (engine.currentMode == Engine::Mode::EDIT && !io.WantCaptureMouse && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        float panelWidth     = io.DisplaySize.x * controlPanelWidthRatio;
        float sceneTabHeight = io.DisplaySize.y / 18.0f;
        float panelHeight    = io.DisplaySize.y - (io.DisplaySize.y * UI::assetConsoleHeightRatio);
        Vector2 mouseScreen = GetMousePosition();
        if (mouseScreen.x > panelWidth && mouseScreen.y > sceneTabHeight && mouseScreen.y < panelHeight) {
            Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());
            int tileSize = grid.GetTileSize();
            int gridX = static_cast<int>(worldPos.x) / tileSize;
            int gridY = static_cast<int>(worldPos.y) / tileSize;

            // If bulk edit is active, update the end position
            if (bulkEditActive) {
                bulkEditEndX = gridX;
                bulkEditEndY = gridY;
            }

            if (Scene *scene = engine.GetCurrentScene()) {
                TileMap &tileMap = scene->GetTileMap();
                if (tileMap.HasTile(gridX, gridY)) {
                    const auto& entities = scene->GetEditModeEntities();
                    bool occupiedByEntity = false;
                    for (const auto& entity : entities) {
                        if (const auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
                            if (player->GetGridX() == contextTileX && player->GetGridY() == contextTileY) {
                                occupiedByEntity = true;
                                break;
                            }
                            if (const auto enemy = dynamic_cast<EnemyEntity*>(entity.get())) {
                                if (enemy->GetGridX() == contextTileX && enemy->GetGridY() == contextTileY) {
                                    occupiedByEntity = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!occupiedByEntity) {
                        contextTileX = gridX;
                        contextTileY = gridY;
                        ImGui::OpenPopup("TileContextMenu");
                    }
                }
            }
        }
    }

    // VISUAL FEEDBACK: Draw selection rectangle in edit mode
    if (bulkEditActive && engine.currentMode == Engine::Mode::EDIT) {
        int minX = std::min(bulkEditStartX, bulkEditEndX);
        int maxX = std::max(bulkEditStartX, bulkEditEndX);
        int minY = std::min(bulkEditStartY, bulkEditEndY);
        int maxY = std::max(bulkEditStartY, bulkEditEndY);

        int tileSize = grid.GetTileSize();
        Camera2D camera = grid.GetCamera();

        // Draw selection rectangle overlay
        BeginMode2D(camera);
        Rectangle selectionRect = {
            static_cast<float>(minX * tileSize),
            static_cast<float>(minY * tileSize),
            static_cast<float>((maxX - minX + 1) * tileSize),
            static_cast<float>((maxY - minY + 1) * tileSize)
        };
        DrawRectangleLinesEx(selectionRect, 3.0f, YELLOW);
        DrawRectangleRec(selectionRect, Fade(YELLOW, 0.2f));
        EndMode2D();
    }

    if (ImGui::BeginPopup("TileContextMenu")) {
        Scene* scene = engine.GetCurrentScene();
        if (!scene) {
            SetDebugMessage("No active scene found.");
            ImGui::EndPopup();
            return;
        }
        TileMap& tileMap = scene->GetTileMap();
        std::map<int, TileData> *tileVecPtr = tileMap.GetTilePtr(contextTileX, contextTileY);

        if (!tileVecPtr || tileVecPtr->empty()) {
            ImGui::Text("No tile at this position");
            ImGui::EndPopup();
            return;
        }
        ImGui::Text("Tile (%d, %d)", contextTileX, contextTileY);
        ImGui::Separator();

        const int currentLayer = engine.GetCurrentTileLayer();
        TileData* tile = nullptr;
        for (auto& t : *tileVecPtr) {
            if (t.second.layer == currentLayer) {
                tile = &t.second;
                break;
            }
        }
        if (!tile && !tileVecPtr->empty()) {
            tile = &(*tileVecPtr)[0];
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Showing tile from layer %d (not active layer %d)", tile->layer, currentLayer);
        }
        if (tile) {
            bool isSolid = tile->isSolid;
            if (ImGui::Checkbox("Solid", &isSolid)) {
                tile->isSolid = isSolid;
                tileMapModified = true;
                SetDebugMessage("[TILE] Collision " + std::string(isSolid ? "ENABLED" : "DISABLED") +
                    " at (" + std::to_string(contextTileX) + ", " +
                    std::to_string(contextTileY) + ") on layer " +
                    std::to_string(tile->layer));
            }

            // BULK COLLISION EDIT FEATURE WITH LIVE PREVIEW
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Bulk Collision Edit");

            if (!bulkEditActive) {
                if (ImGui::Button("Start Selection", ImVec2(-1, 0))) {
                    bulkEditStartX = contextTileX;
                    bulkEditStartY = contextTileY;
                    bulkEditEndX = contextTileX;
                    bulkEditEndY = contextTileY;
                    bulkEditActive = true;
                    SetDebugMessage("[BULK EDIT] Selection started at (" +
                                  std::to_string(contextTileX) + ", " +
                                  std::to_string(contextTileY) + ") - Right-click tiles to expand selection");
                }
                ImGui::TextDisabled("Start a rectangular selection");
            } else {
                // Update end position to current tile
                bulkEditEndX = contextTileX;
                bulkEditEndY = contextTileY;

                int minX = std::min(bulkEditStartX, bulkEditEndX);
                int maxX = std::max(bulkEditStartX, bulkEditEndX);
                int minY = std::min(bulkEditStartY, bulkEditEndY);
                int maxY = std::max(bulkEditStartY, bulkEditEndY);

                // Count actual tiles in selection
                int tilesWithData = 0;
                int emptyPositions = 0;
                for (int y = minY; y <= maxY; y++) {
                    for (int x = minX; x <= maxX; x++) {
                        std::map<int, TileData> *checkTile = tileMap.GetTilePtr(x, y);
                        if (checkTile && !checkTile->empty()) {
                            tilesWithData++;
                        } else {
                            emptyPositions++;
                        }
                    }
                }

                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SELECTION ACTIVE");
                ImGui::Text("From: (%d,%d) To: (%d,%d)",
                           bulkEditStartX, bulkEditStartY,
                           bulkEditEndX, bulkEditEndY);
                ImGui::Text("Grid: %dx%d cells",
                           (maxX - minX + 1), (maxY - minY + 1));
                ImGui::Text("Tiles: %d | Empty: %d", tilesWithData, emptyPositions);
                ImGui::TextDisabled("Right-click more tiles to adjust");

                ImGui::Separator();

                // Enable Collision button
                if (ImGui::Button("Enable Collision", ImVec2(-1, 0))) {
                    int changed = 0;

                    for (int y = minY; y <= maxY; y++) {
                        for (int x = minX; x <= maxX; x++) {
                            std::map<int, TileData> *tileVecPtr = tileMap.GetTilePtr(x, y);
                            if (tileVecPtr && !tileVecPtr->empty()) {
                                bool foundOnLayer = false;
                                for (auto& t : *tileVecPtr) {
                                    if (t.second.layer == currentLayer) {
                                        if (!t.second.isSolid) {
                                            t.second.isSolid = true;
                                            changed++;
                                        }
                                        foundOnLayer = true;
                                        break;
                                    }
                                }
                                if (!foundOnLayer) {
                                    for (auto& t : *tileVecPtr) {
                                        if (!t.second.isSolid) {
                                            t.second.isSolid = true;
                                            changed++;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    tileMapModified = true;
                    SetDebugMessage("[BULK EDIT] Enabled collision on " + std::to_string(changed) + " tiles");
                    bulkEditActive = false;
                }

                // Disable Collision button
                if (ImGui::Button("Disable Collision", ImVec2(-1, 0))) {
                    int changed = 0;

                    for (int y = minY; y <= maxY; y++) {
                        for (int x = minX; x <= maxX; x++) {
                            std::map<int, TileData> *tile_vec_ptr = tileMap.GetTilePtr(x, y);
                            if (tile_vec_ptr && !tile_vec_ptr->empty()) {
                                bool foundOnLayer = false;
                                for (auto& t : *tile_vec_ptr) {
                                    if (t.second.layer == currentLayer) {
                                        if (t.second.isSolid) {
                                            t.second.isSolid = false;
                                            changed++;
                                        }
                                        foundOnLayer = true;
                                        break;
                                    }
                                }
                                if (!foundOnLayer) {
                                    for (auto& t : *tile_vec_ptr) {
                                        if (t.second.isSolid) {
                                            t.second.isSolid = false;
                                            changed++;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    tileMapModified = true;
                    SetDebugMessage("[BULK EDIT] Disabled collision on " + std::to_string(changed) + " tiles");
                    bulkEditActive = false;
                }

                if (ImGui::Button("Cancel Selection", ImVec2(-1, 0))) {
                    bulkEditActive = false;
                    SetDebugMessage("[BULK EDIT] Selection cancelled");
                }
            }

            ImGui::Separator();
            bool isSceneSwitch = tile->isSceneSwitcher;
            if (ImGui::Checkbox("Scene Switch", &isSceneSwitch)) {
                tile->isSceneSwitcher = isSceneSwitch;
                if (isSceneSwitch) {
                    if (tile->triggerKey == 0) {
                        tile->triggerKey = KEY_E;
                    }
                    auto& scenes = engine.GetAllScenes();
                    if (tile->targetSceneID.empty() && !scenes.empty()) {
                        tile->targetSceneID = scenes.begin()->first;
                    }
                }
                tileMapModified = true;
            }
            if (tile->isSceneSwitcher) {
                struct KeyOption {
                    const char* name;
                    int key;
                };
                static KeyOption keyOptions[] = {
                    {"E", KEY_E},
                    {"F", KEY_F},
                    {"G", KEY_G},
                    {"Q", KEY_Q}
                };
                constexpr int keyOptionCount = sizeof(keyOptions) / sizeof(KeyOption);
                int currentKeyIndex = 0;
                if (tile->triggerKey == 0) {
                    tile->triggerKey = keyOptions[0].key;
                    currentKeyIndex = 0;
                }
                else {
                    for (int i = 0; i < keyOptionCount; i++) {
                        if (keyOptions[i].key == tile->triggerKey) {
                            currentKeyIndex = i;
                            break;
                        }
                    }
                }
                const char* keyNames[keyOptionCount];
                for (int i = 0; i < keyOptionCount; i++) {
                    keyNames[i] = keyOptions[i].name;
                }
                if (ImGui::Combo("Trigger Key", &currentKeyIndex, keyNames, keyOptionCount)) {
                    tile->triggerKey = keyOptions[currentKeyIndex].key;
                    tileMapModified = true;
                }
                auto& scenes = engine.GetAllScenes();
                if (scenes.size() <= 1) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No other scenes available");
                }
                else {
                    std::vector<std::string> sceneNames;
                    std::vector<std::string> sceneIDs;
                    sceneNames.reserve(scenes.size());
                    sceneIDs.reserve(scenes.size());
                    for (auto& [id, sPtr] : scenes) {
                        sceneNames.emplace_back(sPtr->GetName());
                        sceneIDs.emplace_back(id);
                    }
                    int currentSceneIndex = 0;
                    if (tile->targetSceneID.empty()) {
                        tile->targetSceneID = sceneIDs[0];
                    }
                    else {
                        for (int i = 0; i < static_cast<int>(sceneIDs.size()); i++) {
                            if (sceneIDs[i] == tile->targetSceneID) {
                                currentSceneIndex = i;
                                break;
                            }
                        }
                    }
                    std::vector<const char*> sceneLabels;
                    sceneLabels.reserve(scenes.size());
                    for (const auto& n : sceneNames) {
                        sceneLabels.emplace_back(n.c_str());
                    }
                    if (ImGui::Combo("Target Scene", &currentSceneIndex, sceneLabels.data(), static_cast<int>(sceneLabels.size()))) {
                        tile->targetSceneID = sceneIDs[currentSceneIndex];
                        tileMapModified = true;
                    }
                }
            }
        }
        ImGui::EndPopup();
    }
}

void UI::RenderLayerVisibilityControls(Engine &engine) {
    // Simple numbered buttons grid with wrapping
    int currentLayers = engine.GetTotalLayers();
    ImGui::Text("Layers:");
    const float buttonSize = 30.0f;
    float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

    // Calculate how many buttons fit per row
    float availableWidth = ImGui::GetContentRegionAvail().x;
    int buttonsPerRow = std::max(1, static_cast<int>(availableWidth + itemSpacing) / static_cast<int>(buttonSize + itemSpacing));

    for (int i = 0; i < currentLayers; i++) {
        // Add wrapping - start new row when we hit the limit
        if (i > 0 && (i % buttonsPerRow) != 0) {
            ImGui::SameLine();
        }

        bool isVisible = engine.IsLayerVisible(i);
        int currentLayer = engine.GetCurrentTileLayer();
        bool isActive = (i == currentLayer);

        // Style the button based on state
        if (!isVisible) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        } else if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        }

        // Create button
        std::string buttonLabel = std::to_string(i);
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(buttonSize, buttonSize))) {
            // Left click: set as active layer
            engine.SetCurrentTileLayer(i);
        }

        if (!isVisible || isActive) {
            ImGui::PopStyleColor();
        }

        // Right-click context menu for each layer
        if (ImGui::BeginPopupContextItem()) {
            // Layer index in the popup title
            ImGui::Text("Layer %d", i);
            ImGui::Separator();

            if (ImGui::MenuItem(isVisible ? "Hide" : "Show", nullptr, isVisible)) {
                engine.ToggleLayerVisibility(i);
            }

            if (ImGui::MenuItem("Set as Active")) {
                engine.SetCurrentTileLayer(i);
            }

            ImGui::Separator();

            bool canRemoveThisLayer = engine.CanRemoveSpecificLayer(i);
            if (!canRemoveThisLayer) {
                ImGui::BeginDisabled();
            }
            if (ImGui::MenuItem(TR("delete_layer"))) {
                engine.RemoveSpecificLayer(i);
            }
            if (!canRemoveThisLayer) {
                ImGui::EndDisabled();
            }
            if (ImGui::IsItemHovered() && !canRemoveThisLayer) {
                ImGui::SetTooltip("Cannot delete: Layer contains tiles/assets or is the last layer");
            }

            ImGui::EndPopup();
        }

        // Tooltip on hover
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Layer %d", i);
            ImGui::Text("Visible: %s", isVisible ? "Yes" : "No");
            ImGui::Text("Active: %s", isActive ? "Yes" : "No");
            ImGui::Text("Left click: Set active");
            ImGui::Text("Right click: Options");
            ImGui::EndTooltip();
        }
    }

    if (ImGui::Button(TR("add_layer"), ImVec2(100, 0))) {
        engine.AddLayer();
    }
}

std::string UI::GetAssetTypeName(const AssetType type) {
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
            const ImVec2 previewSize = ImVec2(64, 64);
            ImVec2 uv0 = ImVec2(selectedRect.x / static_cast<float>(tileset->texture.width),
                               selectedRect.y / static_cast<float>(tileset->texture.height));
            ImVec2 uv1 = ImVec2((selectedRect.x + selectedRect.width) / static_cast<float>(tileset->texture.width),
                               (selectedRect.y + selectedRect.height) / static_cast<float>(tileset->texture.height));

            ImGui::Image((ImTextureID)static_cast<uintptr_t>(tileset->texture.id), previewSize, uv0, uv1);
        } else {
            ImGui::Text("No tile selected - click on a tile below");
        }

        ImGui::Separator();

        // Display the tileset grid for selection
        RenderTilesetGrid(engine, tileset);

    }
    ImGui::End();
}

void UI::RenderTilesetGrid(Engine& engine, Asset* tileset) {
    if (!tileset->loaded || tileset->subSprites.empty()) {
        ImGui::Text("Tileset not loaded or no sub-sprites available");
        return;
    }

    ImGui::Text("Click on a tile to select it:");

    // Calculate grid layout
    const int textureWidth = tileset->texture.width;
    const int textureHeight = tileset->texture.height;
    const int tileWidth = tilesetTileWidth;
    const int tileHeight = tilesetTileHeight;

    const int columns = textureWidth / tileWidth;
    const int rows = textureHeight / tileHeight;

    constexpr float zoom = 2.0f; // Zoom factor for better visibility
    const float displayTileSize = static_cast<float>(tileWidth) * zoom;

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
            ImVec2 uv0 = ImVec2(tileRect.x / static_cast<float>(textureWidth), tileRect.y / static_cast<float>(textureHeight));
            ImVec2 uv1 = ImVec2((tileRect.x + tileRect.width) / static_cast<float>(textureWidth),
                               (tileRect.y + tileRect.height) / static_cast<float>(textureHeight));

            // Display the tile
            ImGui::Image((ImTextureID)static_cast<uintptr_t>(tileset->texture.id),
                        ImVec2(displayTileSize, displayTileSize), uv0, uv1);

            // Tile selection
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                selectedTileIndex = tileIndex;
                selectedTileCoords = {static_cast<float>(x), static_cast<float>(y)};
                engine.currentTool = Engine::ToolState::PLACING_TILE;
                engine.tileToolState.tileset = tileset;
                engine.tileToolState.selectedTileIndex = tileIndex;
                engine.tileToolState.isPlacingTile = true;

                SetDebugMessage("[TILE] Selected and ready to place tile #" + std::to_string(tileIndex));
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
    float panelWidth = io.DisplaySize.x * controlPanelWidthRatio;
    float consoleHeight = io.DisplaySize.y * assetConsoleHeightRatio;
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - panelWidth, io.DisplaySize.y - consoleHeight));
    ImGui::SetNextWindowPos(ImVec2(panelWidth, 0));
    //std::cout << io.DisplaySize.x << " " << io.DisplaySize.y << std::endl;

    if (ImGui::Begin("Play Mode", &engine.playModeWindowOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        const ImVec2 contentSize = ImGui::GetContentRegionAvail();
        const std::string fps = "FPS: " + std::to_string(io.Framerate);
        const char* fpsText = fps.c_str();
        ImGui::Text(fpsText);
        // Calculate UV coordinates to fix inversion
        const ImVec2 uv0 = ImVec2(0, 1);
        const ImVec2 uv1 = ImVec2(1, 0);

        // Maintain aspect ratio
        const float texW = static_cast<float>(engine.playModeTexture.texture.width);
        const float texH = static_cast<float>(engine.playModeTexture.texture.height);

        // Compute the maximum integer scale that fits in the window
        const float scaleX = std::floor(contentSize.x / texW);
        const float scaleY = std::floor(contentSize.y / texH);
        const float scale = std::max(1.0f, std::min(scaleX, scaleY));

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
            (ImTextureID)static_cast<uintptr_t>(engine.playModeTexture.texture.id),
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
        ImGui::Text("Removal Tools:");

        ImGui::SameLine();
        if (ImGui::Button("Remove Player", ImVec2(100, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_PLAYER) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_PLAYER;
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove Enemy", ImVec2(100, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_ENEMY) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_ENEMY;
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove Tile", ImVec2(100, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_TILE) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_TILE;
        }

        ImGui::Separator();
        ImGui::Text("Layer Controls");

        int currentLayer = engine.GetCurrentTileLayer();
        int totalLayers = engine.GetTotalLayers();
        ImGui::Text("Active Layer: %d/%d", currentLayer + 1, totalLayers);

        if (ImGui::Button("Layer Up")) {
            int newLayer = (currentLayer - 1 + totalLayers) % totalLayers;
            engine.SetCurrentTileLayer(newLayer);
        }
        ImGui::SameLine();
        if (ImGui::Button("Layer Down")) {
            int newLayer = (currentLayer + 1) % totalLayers;
            engine.SetCurrentTileLayer(newLayer);
        }

        // Show layer management controls
        RenderLayerVisibilityControls(engine);

        ImGui::Separator();
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
            case Engine::ToolState::PLACING_ASSET: ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Placing Asset"); break;
            case Engine::ToolState::REMOVING_ASSET: ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Removing Asset"); break;
        }

        // Show enemy count
        ImGui::Text("Enemies: %d/10", engine.GetEnemyCount());

        if (ImGui::Button(TR("clear_tool"), ImVec2(120, 0))) {
            engine.ResetTool();
        }
        ImGui::Separator();
        ImGui::Text("Project:");
        static char filenameBuf[256] = "project.json";
        ImGui::InputText("Path:", filenameBuf, sizeof(filenameBuf));
        if (ImGui::Button(TR("save_project"), ImVec2(120, 0))) {
            SaveLoad::SaveProject(engine,std::string(filenameBuf));
            SetDebugMessage("[PROJECT] Saved project to " + std::string(filenameBuf));
        }
        ImGui::SameLine();
        if (ImGui::Button(TR("load_project"), ImVec2(120, 0))) {
            SaveLoad::LoadProject(engine, std::string(filenameBuf));
            SetDebugMessage("[PROJECT] Loaded project from " + std::string(filenameBuf));
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
    resolutionLabels.reserve(resolutions.size());
for (const auto& res : resolutions) {
        resolutionLabels.push_back(res.name);
    }

    if (ImGui::Combo("##Resolution", &currentIndex, resolutionLabels.data(), static_cast<int>(resolutionLabels.size()))) {
        // Use public setter method
        engine.SetSelectedResolutionIndex(currentIndex);
    }

    // Display current resolution info
    if (currentIndex >= 0 && currentIndex < resolutions.size()) {
        auto& currentRes = resolutions[currentIndex];
        ImGui::Text("Current: %dx%d (%.1f:1)",
                   currentRes.width, currentRes.height,
                   static_cast<float>(currentRes.width) / static_cast<float>(currentRes.height));

        // Aspect ratio info
        const float aspect = static_cast<float>(currentRes.width) / static_cast<float>(currentRes.height);
        const char* aspectName = "Custom";
        if (abs(aspect - 16.0f/9.0f) < 0.01f) aspectName = "16:9";
        else if (abs(aspect - 4.0f/3.0f) < 0.01f) aspectName = "4:3";
        else if (abs(aspect - 1.0f) < 0.01f) aspectName = "1:1";

        ImGui::Text("Aspect Ratio: %s", aspectName);
    }
}

void UI::RenderNewSceneTab(Engine& engine) {
    ImGuiIO& io = ImGui::GetIO();
    float panelWidth = io.DisplaySize.x * controlPanelWidthRatio;

    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - panelWidth, io.DisplaySize.y / 18.0f));
    ImGui::SetNextWindowPos(ImVec2(panelWidth, 0));

    ImGui::Begin("Scenes", nullptr,
                 ImGuiWindowFlags_NoMove
                 | ImGuiWindowFlags_NoResize);

    const int sceneCount        = engine.GetSceneCount();
    int       currentSceneIndex = engine.GetCurrentSceneIndex();

    // "+" button to create new scene
    if (ImGui::Button("+", ImVec2(30.0f,
                                  ImGui::GetContentRegionAvail().y))) {
        engine.CreateNewScene();
        SetDebugMessage("[SCENE] Created new scene");
    }

    ImGui::SameLine();

    static int  lastPopupSceneIndex = -1;
    static char renameBuf[64] = "";

    // One tab per scene
    for (int i = 0; i < sceneCount; i++) {
        ImGui::PushID(i);

        std::string sceneName = engine.GetSceneName(i);
        std::string sceneID   = engine.GetSceneID(i);
        bool isActive = (i == currentSceneIndex);
        bool deleteThis = false;

        // Highlight active scene
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button,         ImVec4(0.2f, 0.2f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.3f, 0.3f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.1f, 0.1f, 0.7f, 1.0f));
        }

        ImVec2 tabSize(120.0f, ImGui::GetContentRegionAvail().y);
        if (ImGui::Button(sceneName.c_str(), tabSize)) {
            engine.SetCurrentSceneIndex(i);
            currentSceneIndex = i;
            SetDebugMessage("[SCENE] Switched to scene: " + sceneName);
        }

        // Pop style colors if we pushed them
        if (isActive) {
            ImGui::PopStyleColor(3);
        }

        if (ImGui::BeginPopupContextItem("SceneContext")) {

            // Initialize rename buffer when we open for a different scene
            if (lastPopupSceneIndex != i) {
                std::string currName = engine.GetSceneName(i);
                std::snprintf(renameBuf, sizeof(renameBuf), "%s", currName.c_str());
                lastPopupSceneIndex = i;
            }

            ImGui::Text("Scene options");
            ImGui::Separator();

            // Rename
            ImGui::Text("Name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::InputText("##SceneRename", renameBuf, sizeof(renameBuf));

            if (ImGui::Button("Apply rename")) {
                std::string newName = renameBuf;
                if (!newName.empty()) {
                    engine.RenameScene(i, newName);
                    SetDebugMessage("[SCENE] Renamed scene to: " + newName);
                }
            }

            ImGui::Separator();

            // Starting scene checkbox
            std::string startingID = engine.GetStartingSceneID();
            bool isStarting = (startingID == sceneID);

            if (ImGui::Checkbox("Starting scene", &isStarting)) {
                if (isStarting) {
                    engine.SetStartingSceneID(sceneID);
                    SetDebugMessage("[SCENE] Set as starting scene");
                } else {
                    engine.SetStartingSceneID(""); // allow "no starting scene"
                    SetDebugMessage("[SCENE] Cleared starting scene");
                }
            }

            ImGui::EndPopup();
        }

        // Small "x" delete button (only if more than one scene)
        if (sceneCount > 1) {
            ImGui::SameLine(0.0f, 2.0f);
            if (ImGui::SmallButton("x")) {
                deleteThis = true;
            }
        }

        ImGui::SameLine();
        ImGui::PopID();

        // Handle deletion *after* all pushes/pops are balanced
        if (deleteThis) {
            engine.DeleteScene(i);
            SetDebugMessage("[SCENE] Deleted scene: " + sceneName);
            break;
        }
    }
    ImGui::End();
}

void UI::ClearSelectedEntity() {
    selectedSceneEntity = nullptr;
}