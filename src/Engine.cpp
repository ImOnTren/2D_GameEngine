#include "Engine.h"

using namespace std;

Engine::Engine() : playerManager(grid), enemyManager(grid) {
    playerCamera.offset = {0, 0};
    playerCamera.target = {0, 0};
    playerCamera.rotation = 0.0f;
    playerCamera.zoom = 1.0f;
    playModeTexture = {0};
    playModeWindowOpen = false;
    layerVisibility.resize(MAX_LAYERS, true);
    HandleSceneCreation();
    UpdatePlayerCamera();
    lastPlacedTileX = -999;
    lastPlacedTileY = -999;
    isPlacingTiles = false;
}

Engine::~Engine(){
    assetManager.UnloadAllAssets();
    if (testAnimationSet.textureLoaded) {
        UnloadTexture(testAnimationSet.texture);
    }
}

void Engine::Init() {
    InitWindow(800, 400, "Game Engine");
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(144);
    rlImGuiSetup(true);

    playModeTexture = LoadRenderTexture(availableResolutions[selectedResolutionIndex].width,
                                       availableResolutions[selectedResolutionIndex].height);

    LoadAssets();

    AssetPersistence::LoadAssetDefinitions(assetManager, "assets.json");
}

void Engine::LoadAssets()
{
    assetManager.LoadAsset("grass_tileset", "Grass_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Tileset Grass Spring.png", 16, 16);
    assetManager.LoadAsset("winter_tileset", "Winter_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Tileset Grass Winter.png", 16, 16);
    assetManager.LoadAsset("water_tileset", "Water_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Water Ground animations tiles.png", 16, 16);
    assetManager.LoadAsset("caves_tileset", "Caves_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Caves.png", 16, 16);
    assetManager.LoadAsset("tilled_soil_tileset", "Tilled_soil_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Tilled Soil and wet soil.png", 16, 16);
    assetManager.LoadAsset("all_props_tileset", "all_props_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/ALL props seasons.png", 16, 16);
    assetManager.LoadAsset("water_ground_tileset", "water_ground_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Water Ground animations tiles.png", 16, 16);
    assetManager.LoadAsset("ground_road_tileset", "road_tileset", "Tileset", "../src/assets/Exterior/Road.png", 16, 16);
    assetManager.LoadAsset("fence_wood_tileset", "Fence_wood_tileset", "Tileset", "../src/assets/Exterior/Fence and Bridge/Fence Wood.png", 16, 16);
    assetManager.LoadAsset("fence_stone_tileset", "Fence_stone_tileset", "Tileset", "../src/assets/Exterior/Fence and Bridge/Fence Stone.png", 16, 16);
    assetManager.LoadAsset("water_ground_tileset", "water_ground_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Water Ground animations tiles.png", 16, 16);
    assetManager.LoadAsset("broken_house8", "Broken_House_8_texture", "Static Texture", "../src/assets/Exterior/Houses/8.png");
    assetManager.LoadAsset("broken_house_resized8", "Broken_House_8_resized_texture", "Static Texture", "../src/assets/Exterior/Houses/8 - resized.png");
    assetManager.LoadAsset("broken_house7", "Broken_House_7_texture", "Static Texture", "../src/assets/Exterior/Houses/7.png");
    assetManager.LoadAsset("broken_house6", "Broken_House_6_texture", "Static Texture", "../src/assets/Exterior/Houses/6.png");
    assetManager.LoadAsset("broken_house5", "Broken_House_5_texture", "Static Texture", "../src/assets/Exterior/Houses/5.png");
    assetManager.LoadAsset("broken_house4", "Broken_House_4_texture", "Static Texture", "../src/assets/Exterior/Houses/4.png");
    assetManager.LoadAsset("broken_house3", "Broken_House_3_texture", "Static Texture", "../src/assets/Exterior/Houses/3.png");
    assetManager.LoadAsset("broken_house2", "Broken_House_2_texture", "Static Texture", "../src/assets/Exterior/Houses/2.png");
    assetManager.LoadAsset("broken_house1", "Broken_House_1_texture", "Static Texture", "../src/assets/Exterior/Houses/1.png");
    assetManager.LoadAsset("water_house", "water_house_texture", "Static Texture", "../src/assets/Exterior/Houses/NPCS Houses/1.png");
    assetManager.LoadAsset("outside_table", "outside_table_texture", "Static Texture", "../src/assets/Exterior/Table.png");
    assetManager.LoadAsset("ice_cream_car", "ice_cream_car_texture", "Static Texture", "../src/assets/Exterior/ice cream car.png");
    assetManager.LoadAsset("birch_tree", "birch_tree_texture", "Static Texture", "../src/assets/Farm/Tree/Common/No Shadow/Birch Tree single.png");
    assetManager.LoadAsset("mahogany_tree", "mahogany_tree_texture", "Static Texture", "../src/assets/Farm/Tree/Common/No Shadow/Mahogany Tree single.png");
}


bool Engine::IsMouseOverUI() const {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

void Engine::HandleEditModeInput() {
    if (currentMode != Mode::EDIT) return;

    switch (currentTool) {
        case ToolState::PLACING_PLAYER:
            HandlePlayerPlacement();
            break;
        case ToolState::REMOVING_PLAYER:
            HandlePlayerRemoval();
            break;
        case ToolState::PLACING_ENEMY:
            HandleEnemyPlacement();
            break;
        case ToolState::REMOVING_ENEMY:
            HandleEnemyRemoval();
            break;
        case ToolState::PLACING_TILE:
            HandleTilePlacement();
            break;
        case ToolState::REMOVING_TILE:
            HandleTileRemoval();
            break;
        case ToolState::PLACING_ASSET:
            UpdateAssetPlacementPreview();
            HandleAssetPlacement();
            break;
        case ToolState::REMOVING_ASSET:
            HandleAssetRemoval();
            break;
        case ToolState::NONE:
            if (!IsMouseOverUI()) {
                grid.Update();
            }
            break;
    }
}
void Engine::StartPlayMode() {
    // Check if a valid starting scene is set
    if (startingSceneID.empty() || scenes.find(startingSceneID) == scenes.end()) {
        UI::SetDebugMessage("[WARNING] Cannot start play mode: No starting scene set");
        playModeWindowOpen = false;
        currentMode = Mode::EDIT;
        return;
    }

    // Switch to the starting scene
    currentSceneID = startingSceneID;
    Scene* scene = scenes[startingSceneID].get();

    TileMap& tileMap = scene->GetTileMap();
    auto& editModeEntities = scene->GetEditModeEntities();

    if (playerManager.PlayerExists(editModeEntities)) {
        CreatePlayModeSnapshots();

        playModeTileMap = std::make_unique<TileMap>();
        *playModeTileMap = tileMap;

        playModeWindowOpen = true;
        currentMode = Mode::PLAY;
        UpdatePlayModeCamera();
    } else {
        playModeWindowOpen = false;
        currentMode = Mode::EDIT;
        UI::SetDebugMessage("[WARNING] Cannot start play mode: No player placed in starting scene");
    }
}

void Engine::StopPlayMode() {
    Scene* scene = GetCurrentScene();
    if (scene) {
        auto& playModeSnapshots = scene->GetPlayModeSnapshots();
        playModeSnapshots.clear();
    }
    playModeTileMap.reset();
    playModeWindowOpen = false;
    currentMode = Mode::EDIT;
}

void Engine::CreatePlayModeSnapshots() {
    Scene* scene = GetCurrentScene();
    if (!scene) return;
    auto& playModeSnapshots = scene->GetPlayModeSnapshots();
    playModeSnapshots.clear();
    auto& editModeEntities = scene->GetEditModeEntities();
    for (auto& entity : editModeEntities) {
        playModeSnapshots.push_back(entity->CreateSnapshot());
    }
}

// =======================================
// =          Input Handlers            =
// =======================================
void Engine::HandlePlayerPlacement() {
    if (IsMouseOverUI()) return;
    Scene* scene = GetCurrentScene();
    auto& editModeEntities = scene->GetEditModeEntities();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        playerManager.PlacePlayer(mouseScreen, grid.GetCamera(), editModeEntities);
        UpdateEditModeCamera();
    }
}

void Engine::HandlePlayerRemoval() {
    if (IsMouseOverUI()) return;
    Scene* scene = GetCurrentScene();
    auto& editModeEntities = scene->GetEditModeEntities();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        if (playerManager.RemovePlayer(mouseScreen, grid.GetCamera(), editModeEntities)) {
            UpdateEditModeCamera();
        }
    }
}

void Engine::HandleEnemyPlacement() {
    if (IsMouseOverUI()) return;
    Scene* scene = GetCurrentScene();
    auto& editModeEntities = scene->GetEditModeEntities();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        enemyManager.PlaceEnemy(mouseScreen, grid.GetCamera(), editModeEntities);
    }
}

void Engine::HandleEnemyRemoval() {
    if (IsMouseOverUI()) return;
    Scene* scene = GetCurrentScene();
    auto& editModeEntities = scene->GetEditModeEntities();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        enemyManager.RemoveEnemy(mouseScreen, grid.GetCamera(), editModeEntities);
    }
}
//============================================
//             Tile Handlers                 =
//============================================
void Engine::HandleTilePlacement()
{
    if (IsMouseOverUI()) {
        isPlacingTiles = false;
        return;
    }

    Vector2 mouseScreen = GetMousePosition();
    Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());
    Scene* scene = GetCurrentScene();
    TileMap& tileMap = scene->GetTileMap();

    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(worldPos.x) / tileSize;
    int gridY = static_cast<int>(worldPos.y) / tileSize;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        isPlacingTiles = true;
        lastPlacedTileX = -999;  // Invalid position to force first placement
        lastPlacedTileY = -999;
    }

    // Continue placing tiles while mouse button is held down
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isPlacingTiles)
    {
        if (!grid.IsValidCell(gridX, gridY)) {
            UI::SetDebugMessage("[WARNING] Cannot place tile outside grid bounds.");
            return;
        }
        // Only place if we're on a different tile than last time
        if (gridX != lastPlacedTileX || gridY != lastPlacedTileY)
        {
            TileData tile;
            tile.tileID = tileToolState.tileset->id;
            tile.tileIndex = tileToolState.selectedTileIndex;
            tile.layer = tileToolState.activeLayer;

            tileMap.SetTile(gridX, gridY, tile, tileToolState.activeLayer);

            // Update last placed position
            lastPlacedTileX = gridX;
            lastPlacedTileY = gridY;

            UI::SetDebugMessage("[INFO] Tile placed at (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")");
        }
    }

    // Stop placing when mouse button is released
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        isPlacingTiles = false;
    }
}

void Engine::HandleTileRemoval()
{
    if (IsMouseOverUI()) {
        isPlacingTiles = false;
        return;
    }

    Vector2 mouseScreen = GetMousePosition();
    Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());
    Scene* scene = GetCurrentScene();
    TileMap& tileMap = scene->GetTileMap();

    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(worldPos.x) / tileSize;
    int gridY = static_cast<int>(worldPos.y) / tileSize;

    // Start removing tiles on mouse button press
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        isPlacingTiles = true;
        lastPlacedTileX = -999;
        lastPlacedTileY = -999;
    }

    // Continue removing tiles while mouse button is held down
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isPlacingTiles)
    {
        if (!grid.IsValidCell(gridX, gridY)) {
            UI::SetDebugMessage("[WARNING] Cannot remove tile outside grid bounds.");
            return;
        }
        // Only remove if we're on a different tile than last time
        if (gridX != lastPlacedTileX || gridY != lastPlacedTileY)
        {
            if (tileMap.HasTile(gridX, gridY))
            {
                tileMap.RemoveTileFromLayer(gridX, gridY, tileToolState.activeLayer);
                UI::SetDebugMessage("[INFO] Tile removed at (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")");
            }

            lastPlacedTileX = gridX;
            lastPlacedTileY = gridY;
        }
    }

    // Stop removing when mouse button is released
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        isPlacingTiles = false;
    }
}

void Engine::DrawEditModeTiles()
{
    int tileSize = grid.GetTileSize();
    Scene* scene = GetCurrentScene();
    TileMap& tileMap = scene->GetTileMap();

    for (const auto& [key, tileDataVec] : tileMap.GetAllTiles()) {
        const int x = static_cast<int>(key >> 32);
        const int y = static_cast<int>(key & 0xFFFFFFFF);
        std::vector<TileData> tilesAtPos = tileMap.GetTilesAtPosition(x, y);

        for (const auto& tile : tilesAtPos) {
            if (tile.layer >= 0 && tile.layer < tileToolState.totalLayers) {
                if (!layerVisibility[tile.layer]) {
                    continue; // Skip hidden layers
                }
            } else {
                continue;
            }

            const Asset* asset = assetManager.GetAsset(tile.tileID);
            if (!asset || !asset->loaded) continue;

            const Rectangle sourceRect = assetManager.GetSpecificSprite(asset, tile.tileIndex);
            const Rectangle destRect = {
                static_cast<float>(x * tileSize),
                static_cast<float>(y * tileSize),
                static_cast<float>(tileSize),
                static_cast<float>(tileSize)
            };

            DrawTexturePro(asset->texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
        }
    }
}

void Engine::DrawPlayModeTiles()
{
    if (!playModeTileMap) return;

    const int tileSize = grid.GetTileSize();

    for (const auto& [key, tileData] : playModeTileMap->GetAllTiles()) {
        const int x = static_cast<int>(key >> 32);
        const int y = static_cast<int>(key & 0xFFFFFFFF);
        std::vector<TileData> tilesAtPos = playModeTileMap->GetTilesAtPosition(x, y);

        for (const auto& tile : tilesAtPos) {
            if (tile.layer >= 0 && tile.layer < tileToolState.totalLayers) {
                if (!layerVisibility[tile.layer]) {
                    continue; // Skip hidden layers
                }
            } else {
                continue;
            }

            Asset* asset = assetManager.GetAsset(tile.tileID);
            if (!asset || !asset->loaded) continue;

            const Rectangle sourceRect = assetManager.GetSpecificSprite(asset, tile.tileIndex);
            const Rectangle destRect = {
                static_cast<float>(x * tileSize),
                static_cast<float>(y * tileSize),
                static_cast<float>(tileSize),
                static_cast<float>(tileSize)
            };
            //DrawTextureRec(asset->texture, destRect, {0, 0}, WHITE);
            DrawTexturePro(asset->texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
        }
    }
}

void Engine::DrawHoveredTileLayerInfo() {
    if (currentMode != Mode::EDIT || IsMouseOverUI()) return;

    Vector2 mouseScreen = GetMousePosition();
    Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());

    const int tileSize = grid.GetTileSize();
    const int gridX = static_cast<int>(worldPos.x) / tileSize;
    const int gridY = static_cast<int>(worldPos.y) / tileSize;

    if (!grid.IsValidCell(gridX, gridY)) {
        return;
    }

    Scene* scene = GetCurrentScene();
    if (!scene) return;

    const TileMap& tileMap = scene->GetTileMap();
    const std::vector<TileData> tilesAtPos = tileMap.GetTilesAtPosition(gridX, gridY);

    if (tilesAtPos.empty()) return;

    // Layer colors
    std::vector<Color> layerColors;
    for (int i = 0; i < tileToolState.totalLayers; i++) {
        // Generate distinct colors for each layer
        float hue = static_cast<float>(i) / std::max(1.0f, static_cast<float>(tileToolState.totalLayers - 1));
        Color color = ColorFromHSV(hue * 360.0f, 0.8f, 0.9f);
        color.a = 200;  // Semi-transparent
        layerColors.push_back(color);
    }

    // Draw colored squares in the corner of the hovered cell
    const float cellX = static_cast<float>(gridX * tileSize);
    const float cellY = static_cast<float>(gridY * tileSize);
    const float indicatorSize = static_cast<float>(tileSize) / 5.0f;
    float padding = 2.0f;

    int indicatorCount = 0;
    for (const auto& tile : tilesAtPos) {
        if (tile.layer >= 0 && tile.layer < 5) {
            float offsetX = cellX + padding + (static_cast<float>(indicatorCount) * (indicatorSize + 1));
            float offsetY = cellY + padding;

            // Draw indicator square
            DrawRectangle(
                static_cast<int>(offsetX),
                static_cast<int>(offsetY),
                static_cast<int>(indicatorSize),
                static_cast<int>(indicatorSize),
                layerColors[tile.layer]
            );

            // Highlight current active layer
            if (tile.layer == tileToolState.activeLayer) {
                DrawRectangleLinesEx(
                    {offsetX, offsetY, indicatorSize, indicatorSize},
                    1.0f,
                    WHITE
                );
            }

            indicatorCount++;
        }
    }

    // Draw hover outline on the cell
    DrawRectangleLinesEx(
        {cellX, cellY, static_cast<float>(tileSize), static_cast<float>(tileSize)},
        2.0f,
        YELLOW
    );
}

void Engine::SetCurrentTileLayer(const int layer) {
    if (layer >= 0 && layer < tileToolState.totalLayers) {
        tileToolState.activeLayer = layer;
    }
}

int Engine::GetCurrentTileLayer() const {
    return tileToolState.activeLayer;
}

int Engine::GetTotalLayers() const {
    return tileToolState.totalLayers;
}


void Engine::CycleLayerUp() {
    tileToolState.activeLayer--;
    if (tileToolState.activeLayer < 0) {
        tileToolState.activeLayer = tileToolState.totalLayers - 1;
    }
}

void Engine::CycleLayerDown() {
    tileToolState.activeLayer++;
    if (tileToolState.activeLayer >= MAX_LAYERS) {
        tileToolState.activeLayer = 0;
    }
}

bool Engine::CanRemoveSpecificLayer(int layerIndex) const {
    // Can't remove if we only have 1 layer
    if (tileToolState.totalLayers <= 1) {
        return false;
    }

    // Can't remove invalid layer index
    if (layerIndex < 0 || layerIndex >= tileToolState.totalLayers) {
        return false;
    }

    Scene* scene = const_cast<Engine*>(this)->GetCurrentScene();
    if (!scene) return false;

    // Check if the specified layer has any tiles
    TileMap& tileMap = scene->GetTileMap();
    const auto& allTiles = tileMap.GetAllTiles();

    for (const auto& [key, tileVec] : allTiles) {
        for (const auto& tile : tileVec) {
            if (tile.second.layer == layerIndex) {
                return false;  // Layer has tiles, can't remove
            }
        }
    }

    // Check if the specified layer has any entities
    auto& entities = scene->GetEditModeEntities();
    for (const auto& entity : entities) {
        if (auto* staticEntity = dynamic_cast<StaticEntity*>(entity.get())) {
            if (staticEntity->GetLayer() == layerIndex) {
                return false;  // Layer has entities, can't remove
            }
        }
    }

    return true;
}

void Engine::RemoveSpecificLayer(int layerIndex) {
    if (!CanRemoveSpecificLayer(layerIndex)) {
        UI::SetDebugMessage("[WARNING] Cannot remove layer " + std::to_string(layerIndex) +
                           ". Ensure layer is empty and there's more than 1 layer.");
        return;
    }

    Scene* scene = GetCurrentScene();
    if (!scene) return;

    // Shift all layers above this one down by 1
    TileMap& tileMap = scene->GetTileMap();
    auto& allTiles = tileMap.GetAllTiles();

    // Update tile layers
    for (auto& [key, tileVec] : allTiles) {
        for (auto& tile : tileVec) {
            if (tile.second.layer > layerIndex) {
                tile.second.layer--;  // Shift down
            }
        }
    }

    // Update entity layers
    auto& entities = scene->GetEditModeEntities();
    for (auto& entity : entities) {
        if (auto* staticEntity = dynamic_cast<StaticEntity*>(entity.get())) {
            int entityLayer = staticEntity->GetLayer();
            if (entityLayer > layerIndex) {
                staticEntity->SetLayer(entityLayer - 1);  // Shift down
            }
        }
    }

    // Remove the layer from visibility tracking
    if (layerIndex < static_cast<int>(layerVisibility.size())) {
        layerVisibility.erase(layerVisibility.begin() + layerIndex);
    }

    // Decrease total layer count
    tileToolState.totalLayers--;

    // Adjust active layer if needed
    if (tileToolState.activeLayer == layerIndex) {
        // If we deleted the active layer, move to the one below (or 0)
        tileToolState.activeLayer = std::max(0, layerIndex - 1);
    } else if (tileToolState.activeLayer > layerIndex) {
        // If active layer is above deleted layer, shift it down
        tileToolState.activeLayer--;
    }

    UI::SetDebugMessage("[LAYERS] Removed layer " + std::to_string(layerIndex) +
                       ". Total layers: " + std::to_string(tileToolState.totalLayers));
}

bool Engine::IsLayerVisible(const int layer) const {
    if (layer >= 0 && layer < MAX_LAYERS) {
        return layerVisibility[layer];
    }
    return false;
}

void Engine::SetLayerVisible(const int layer, const bool visible) {
    if (layer >= 0 && layer < MAX_LAYERS) {
        layerVisibility[layer] = visible;
    }
}

void Engine::ToggleLayerVisibility(const int layer) {
    if (layer >= 0 && layer < MAX_LAYERS) {
        layerVisibility[layer] = !layerVisibility[layer];
    }
}

void Engine::SetTotalLayers(const int count) {
    if (count >= 1 && count <= MAX_LAYERS) {
        const int oldCount = tileToolState.totalLayers;
        tileToolState.totalLayers = count;

        // Resize layer visibility vector
        layerVisibility.resize(count, true);

        // If we reduced layers, adjust active layer if needed
        if (tileToolState.activeLayer >= count) {
            tileToolState.activeLayer = count - 1;
        }

        UI::SetDebugMessage("[LAYERS] Changed total layers from " +
                           std::to_string(oldCount) + " to " + std::to_string(count));
    }
}

void Engine::AddLayer() {
    if (tileToolState.totalLayers < MAX_LAYERS) {
        tileToolState.totalLayers++;
        layerVisibility.push_back(true);  // New layer is visible by default

        UI::SetDebugMessage("[LAYERS] Added layer " + std::to_string(tileToolState.totalLayers - 1) +
                           ". Total: " + std::to_string(tileToolState.totalLayers));
    } else {
        UI::SetDebugMessage("[WARNING] Maximum layer limit reached (" +
                           std::to_string(MAX_LAYERS) + ")");
    }
}

bool Engine::CanRemoveLayer() const {
    // Only allow removing if we have more than 1 layer
    if (tileToolState.totalLayers <= 1) {
        return false;
    }

    // Check if the last layer is empty
    Scene* scene = const_cast<Engine*>(this)->GetCurrentScene();
    if (!scene) return false;

    // Check tiles in the last layer
    TileMap& tileMap = scene->GetTileMap();
    const auto& allTiles = tileMap.GetAllTiles();

    int lastLayer = tileToolState.totalLayers - 1;
    for (const auto& [key, tileVec] : allTiles) {
        for (const auto& tile : tileVec) {
            if (tile.second.layer == lastLayer) {
                return false;
            }
        }
    }

    // Check entities in the last layer
    auto& entities = scene->GetEditModeEntities();
    for (const auto& entity : entities) {
        if (auto* staticEntity = dynamic_cast<StaticEntity*>(entity.get())) {
            if (staticEntity->GetLayer() == lastLayer) {
                return false;
            }
        }
    }

    return true;
}

void Engine::RemoveLayer() {
    if (!CanRemoveLayer()) {
        UI::SetDebugMessage("[WARNING] Cannot remove layer. Ensure layer is empty and there's more than 1 layer.");
        return;
    }

    tileToolState.totalLayers--;
    layerVisibility.pop_back();

    // Adjust active layer if it was the removed one
    if (tileToolState.activeLayer >= tileToolState.totalLayers) {
        tileToolState.activeLayer = tileToolState.totalLayers - 1;
    }

    UI::SetDebugMessage("[LAYERS] Removed layer. Total: " +
                       std::to_string(tileToolState.totalLayers));
}

//===============================================
//               Static textures                =
//===============================================
void Engine::HandleAssetPlacement(){
    if (IsMouseOverUI()) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Scene* scene = GetCurrentScene();
        Vector2 mouseScreen = GetMousePosition();
        Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());

        const int tileSize = grid.GetTileSize();
        const int gridCellX = static_cast<int>(worldPos.x) / tileSize;
        const int gridCellY = static_cast<int>(worldPos.y) / tileSize;

        if (!grid.IsValidCell(gridCellX, gridCellY)) {
            UI::SetDebugMessage("[WARNING] Cannot place asset outside grid bounds.");
            return;
        }

        scene->GetEditModeEntities().emplace_back(
            std::make_unique<StaticEntity>(grid, assetToolState.asset, gridCellX, gridCellY, tileToolState.activeLayer)
        );
        UI::SetDebugMessage("[INFO] Placed asset '" + assetToolState.asset->name + "' at (" +
                            std::to_string(static_cast<int>(worldPos.x) / grid.GetTileSize()) + ", " +
                            std::to_string(static_cast<int>(worldPos.y) / grid.GetTileSize()) + ")");
    }
}

void Engine::HandleAssetRemoval() {
    if (IsMouseOverUI()) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Scene* scene = GetCurrentScene();
        if (!scene) return;

        Vector2 mouseScreen = GetMousePosition();
        Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());

        auto& entities = scene->GetEditModeEntities();

        // Iterate through entities to find static entities under the mouse
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (auto* staticEntity = dynamic_cast<StaticEntity*>(it->get())) {
                Rectangle entityBounds = staticEntity->GetBounds();

                if (CheckCollisionPointRec(worldPos, entityBounds)) {
                    UI::SetDebugMessage("[INFO] Removed asset '" +
                                      (staticEntity->GetAsset() ? staticEntity->GetAsset()->name : "Unknown") +
                                      "' at (" + std::to_string(staticEntity->GetGridX()) + ", " +
                                      std::to_string(staticEntity->GetGridY()) + ")");
                    entities.erase(it);
                    return;  // Remove only one asset per click
                }
            }
        }

        UI::SetDebugMessage("[INFO] No asset found at cursor position");
    }
}


void Engine::UpdateAssetPlacementPreview() {
    if (currentTool == ToolState::PLACING_ASSET && assetToolState.asset && !IsMouseOverUI()) {
        Vector2 mouseScreen = GetMousePosition();
        Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());

        const int tileSize = grid.GetTileSize();

        // Calculate the grid cell position
        const int gridX = static_cast<int>(worldPos.x) / tileSize;
        const int gridY = static_cast<int>(worldPos.y) / tileSize;

        if (!grid.IsValidCell(gridX, gridY)) {
            UI::SetDebugMessage("[WARNING] Cannot place asset outside grid bounds.");
            return;
        }

        // Convert back to pixel position for preview
        assetToolState.previewPosition = {
            static_cast<float>(gridX * tileSize),
            static_cast<float>(gridY * tileSize)
        };

        assetToolState.showPreview = true;
    } else {
        // Hide preview when not placing or mouse is over UI
        assetToolState.showPreview = false;
    }
}

void Engine::DrawAssetPlacementPreview() const {
    if (assetToolState.showPreview && assetToolState.asset) {
        Rectangle sourceRect;
        float width, height;

        // Check if this asset has a selected frame
        if (assetToolState.asset->SpriteSourceRect.width > 0 &&
            assetToolState.asset->SpriteSourceRect.height > 0) {
            // Use the selected frame
            sourceRect = assetToolState.asset->SpriteSourceRect;
            width = assetToolState.asset->SpriteSourceRect.width;
            height = assetToolState.asset->SpriteSourceRect.height;
            } else {
                // Use full texture
                sourceRect = {0, 0,
                             static_cast<float>(assetToolState.asset->texture.width),
                             static_cast<float>(assetToolState.asset->texture.height)};
                width = static_cast<float>(assetToolState.asset->texture.width);
                height = static_cast<float>(assetToolState.asset->texture.height);
            }

        const float previewX = assetToolState.previewPosition.x +
                        (static_cast<float>(grid.GetTileSize()) - width) / 2.0f;
        const float previewY = assetToolState.previewPosition.y +
                        (static_cast<float>(grid.GetTileSize()) - height) / 2.0f;
        const Rectangle destRect = {previewX, previewY, width, height};

        DrawTexturePro(assetToolState.asset->texture, sourceRect, destRect, {0, 0}, 0.0f,
                      Fade(WHITE, assetToolState.previewAlpha));
        DrawRectangleLinesEx(destRect, 2.0f, YELLOW);
    }
}

//===============================================
//               Scene Management               =
//===============================================
void Engine::HandleSceneCreation() {
    std::string sceneID = "scene_" + std::to_string(nextSceneId++);
    std::string displayName = "Scene " + std::to_string(nextSceneId - 1);

    auto newScene = std::make_unique<Scene>(sceneID, displayName);
    scenes.emplace(sceneID, std::move(newScene));

    currentSceneID = sceneID;
}

void Engine::HandleSceneDeletion(const int& index) {
    if (scenes.size() <= 1) {
        UI::SetDebugMessage("[WARNING] Cannot delete the last scene");
        return;
    }

    auto it = scenes.begin();
    std::advance(it, index);
    const std::string sceneToDelete = it->first;

    // If deleting the current scene, switch to another scene first
    if (sceneToDelete == currentSceneID) {
        // Find a different scene to switch to
        for (auto& [id, scenePtr] : scenes) {
            if (id != sceneToDelete) {
                currentSceneID = id;
                break;
            }
        }
    }

    // Adjust starting scene index if needed
    if (startingSceneID == sceneToDelete) {
        startingSceneID = "";
        UI::SetDebugMessage("[INFO] Starting scene was deleted, please set a new starting scene");
    }

    scenes.erase(sceneToDelete);
    UI::SetDebugMessage("[INFO] Scene deleted successfully");
}
void Engine::ChangeScene(const std::string& targetSceneID, int spawnGridX, int spawnGridY) {
    auto it = scenes.find(targetSceneID);
    if (it == scenes.end()) {
        UI::SetDebugMessage("[ERROR] Scene with ID '" + targetSceneID + "' not found");
        return;
    }
    currentSceneID = targetSceneID;
    Scene* scene = it->second.get();
    auto& playModeSnapshots = scene->GetPlayModeSnapshots();
    playModeSnapshots.clear();
    auto& editModeEntities = scene->GetEditModeEntities();
    for (auto& entity : editModeEntities) {
        playModeSnapshots.push_back(entity->CreateSnapshot());
    }
    playModeTileMap = std::make_unique<TileMap>();
    *playModeTileMap = scene->GetTileMap();
    PlayerEntity* player = playerManager.FindPlayerEntity(playModeSnapshots);
    if (player) {
        player->PlaceOnGrid(spawnGridX, spawnGridY);
    }
    UpdatePlayModeCamera();
    UI::SetDebugMessage("[SCENE] Changed to scene '" + scene->GetName() + "' (ID: " + targetSceneID + ")");
}

void Engine::HandleSceneSwitchInPlayMode() {
    if (!playModeTileMap) {
        return;
    }
    Scene* scene = GetCurrentScene();
    if (!scene) {
        return;
    }
    auto& playModeSnapshots = scene->GetPlayModeSnapshots();
    PlayerEntity* player = playerManager.FindPlayerEntity(playModeSnapshots);
    if (!player) {
        return;
    }

    int playerGridX = player->GetGridX();
    int playerGridY = player->GetGridY();

    if (!playModeTileMap->HasTile(playerGridX, playerGridY)) {
        return;
    }

    std::vector<TileData> tilesAtPos = playModeTileMap->GetTilesAtPosition(playerGridX, playerGridY);
    for (const auto& tile : tilesAtPos) {
        if (tile.tileID.empty()) {
            continue;
        }
        if (!tile.isSceneSwitcher) {
            continue;
        }
        if (tile.triggerKey == 0) {
            continue;
        }
        if (IsKeyPressed(tile.triggerKey)) {
            if (tile.targetSceneID.empty()) {
                UI::SetDebugMessage("[ERROR] Tile at (" + std::to_string(playerGridX) + ", " + std::to_string(playerGridY) + ") has no target scene ID set");
                return;
            }
            ChangeScene(tile.targetSceneID, playerGridX, playerGridY);
            return;
        }
    }
}

// =======================================
// =            Camera Logic             =
// =======================================
void Engine::UpdateEditModeCamera() {
    Scene* scene = GetCurrentScene();
    auto& editModeEntities = scene->GetEditModeEntities();
    PlayerEntity* player = playerManager.FindPlayerEntity(editModeEntities);
    if (!player) {
        scene->SetEditModeCameraArea({0,0,0,0});
        return;
    }

    int width = availableResolutions[selectedResolutionIndex].width;
    int height = availableResolutions[selectedResolutionIndex].height;

    Vector2 playerPos = player->GetPosition();
    Vector2 playerSize = player->GetSize();

    Vector2 playerCenter = {
        playerPos.x + playerSize.x / 2.0f,
        playerPos.y + playerSize.y / 2.0f
    };

    Rectangle editModeCameraArea = {
        playerCenter.x - width / 2.0f,
        playerCenter.y - height / 2.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };
    scene->SetEditModeCameraArea(editModeCameraArea);
}

void Engine::UpdatePlayerCamera() {
    if (currentMode == Mode::PLAY) {
        UpdatePlayModeCamera();
    } else {
        UpdateEditModeCamera();
    }
}

void Engine::UpdatePlayModeCamera() {
    Scene* scene = GetCurrentScene();
    auto& playModeSnapshots = scene->GetPlayModeSnapshots();
    PlayerEntity* player = playerManager.FindPlayerEntity(playModeSnapshots);
    if (!player) {
        scene->SetPlayModeCameraArea({0,0,0,0});
        return;
    }

    int width = availableResolutions[selectedResolutionIndex].width;
    int height = availableResolutions[selectedResolutionIndex].height;

    Vector2 playerPos = player->GetPosition();
    Vector2 playerSize = player->GetSize();

    Vector2 playerCenter = {
        playerPos.x + playerSize.x / 2.0f,
        playerPos.y + playerSize.y / 2.0f
    };

    // Snap camera target to whole pixels for pixel-perfect rendering
    playerCenter.x = std::round(playerCenter.x);
    playerCenter.y = std::round(playerCenter.y);

    Rectangle playModeCameraArea = {
        playerCenter.x - width / 2.0f,
        playerCenter.y - height / 2.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };
    scene->SetPlayModeCameraArea(playModeCameraArea);

    playerCamera.target = playerCenter;
    playerCamera.offset = {width / 2.0f, height / 2.0f};
    playerCamera.rotation = 0.0f;
    playerCamera.zoom = 1.0f;

    if (playModeTexture.texture.width != width || playModeTexture.texture.height != height) {
        if (playModeTexture.id != 0) {
            UnloadRenderTexture(playModeTexture);
        }
        playModeTexture = LoadRenderTexture(width, height);

        // Pixel-perfect for the play-mode render texture
        SetTextureFilter(playModeTexture.texture, TEXTURE_FILTER_POINT);
    }
}

// =======================================
bool Engine::HasPlayer() {
    Scene* scene = GetCurrentScene();
    auto& editModeEntities = scene->GetEditModeEntities();
    return playerManager.PlayerExists(editModeEntities);
}

int Engine::GetEnemyCount() {
    Scene* scene = GetCurrentScene();
    auto& editModeEntities = scene->GetEditModeEntities();
    return enemyManager.GetEnemyCount(editModeEntities);
}

void Engine::ResolveCollisionInPlayMode() {
    if (!playModeTileMap) {
        return;
    }

    Scene* scene = GetCurrentScene();
    if (!scene) {
        return;
    }

    auto& playModeSnapshots = scene->GetPlayModeSnapshots();
    PlayerEntity* player = playerManager.FindPlayerEntity(playModeSnapshots);
    if (!player) {
        return;
    }

    if (!player->IsCollisionEnabled()) {
        return;
    }

    Vector2 previousPlayerPos = player->GetPreviousPosition();
    Vector2 currentPlayerPos = player->GetPosition();

    float dx = currentPlayerPos.x - previousPlayerPos.x;
    float dy = currentPlayerPos.y - previousPlayerPos.y;

    Rectangle playerBounds = player->GetBounds();
    float width = playerBounds.width;
    float height = playerBounds.height;
    Vector2 playerHitboxOffset = player->GetCollisionOffset();

    int tileSize = grid.GetTileSize();

    auto collidesAt = [&](float x, float y, const Entity* ignoreEntity = nullptr) -> bool {
        Rectangle r{
            x + playerHitboxOffset.x,
            y + playerHitboxOffset.y,
            width,
            height
        };

        int startX = static_cast<int>(r.x) / tileSize;
        int startY = static_cast<int>(r.y) / tileSize;
        int endX = static_cast<int>(r.x + r.width - 1) / tileSize;
        int endY = static_cast<int>(r.y + r.height - 1) / tileSize;

        for (int ty = startY; ty <= endY; ++ty) {
            for (int tx = startX; tx <= endX; ++tx) {
                if (!playModeTileMap->HasTile(tx, ty)) {
                    continue;
                }

                std::vector<TileData> tilesAtPos = playModeTileMap->GetTilesAtPosition(tx, ty);
                for (const auto& tile : tilesAtPos) {
                    if (!tile.isSolid) {
                        continue;
                    }

                    Rectangle tileRect{
                        static_cast<float>(tx * tileSize),
                        static_cast<float>(ty * tileSize),
                        static_cast<float>(tileSize),
                        static_cast<float>(tileSize)
                    };

                    if (CheckCollisionRecs(r, tileRect)) {
                        return true;
                    }
                }
            }
        }

        for (auto& entity : playModeSnapshots) {
            if (entity.get() == player) {
                continue;
            }

            if (entity.get() == ignoreEntity) {
                continue;
            }

            if (!entity->IsCollisionEnabled()) {
                continue;
            }

            Rectangle entityBounds = entity->GetBounds();
            if (CheckCollisionRecs(r, entityBounds)) {
                return true;
            }
        }

        return false;
    };

    // -------------------------------------------------
    // 1) DEPENETRATION / UNSTUCK PASS
    // If the player is already overlapping enemies, push
    // them out before the normal movement resolution.
    // -------------------------------------------------
    {
        Rectangle currentBounds = player->GetBounds();
        Vector2 correctedPos = player->GetPosition();

        for (auto& entity : playModeSnapshots) {
            if (entity.get() == player) {
                continue;
            }

            if (!entity->IsCollisionEnabled()) {
                continue;
            }

            Rectangle other = entity->GetBounds();

            if (!CheckCollisionRecs(currentBounds, other)) {
                continue;
            }

            float overlapLeft   = (currentBounds.x + currentBounds.width) - other.x;
            float overlapRight  = (other.x + other.width) - currentBounds.x;
            float overlapTop    = (currentBounds.y + currentBounds.height) - other.y;
            float overlapBottom = (other.y + other.height) - currentBounds.y;

            float pushX = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
            float pushY = (overlapTop < overlapBottom) ? -overlapTop : overlapBottom;

            // Push along the axis of least penetration
            if (fabsf(pushX) < fabsf(pushY)) {
                float testX = correctedPos.x + pushX;
                if (!collidesAt(testX, correctedPos.y, entity.get())) {
                    correctedPos.x = testX;
                }
            } else {
                float testY = correctedPos.y + pushY;
                if (!collidesAt(correctedPos.x, testY, entity.get())) {
                    correctedPos.y = testY;
                }
            }

            player->SetWorldPositionAndUpdateGrid(correctedPos);
            currentBounds = player->GetBounds();
        }
    }

    // Re-read after possible depenetration
    previousPlayerPos = player->GetPreviousPosition();
    currentPlayerPos = player->GetPosition();

    dx = currentPlayerPos.x - previousPlayerPos.x;
    dy = currentPlayerPos.y - previousPlayerPos.y;

    if (dx == 0.0f && dy == 0.0f) {
        return;
    }

    Vector2 resolvedPos = previousPlayerPos;
    bool xBlocked = false;
    bool yBlocked = false;

    if (dx != 0.0f) {
        float candX = previousPlayerPos.x + dx;
        if (!collidesAt(candX, previousPlayerPos.y)) {
            resolvedPos.x = candX;
        } else {
            xBlocked = true;
        }
    }

    if (dy != 0.0f) {
        float candY = previousPlayerPos.y + dy;
        if (!collidesAt(resolvedPos.x, candY)) {
            resolvedPos.y = candY;
        } else {
            yBlocked = true;
        }
    }

    player->SetWorldPositionAndUpdateGrid(resolvedPos);

    Vector2 playerVelocity = player->GetVelocity();
    playerVelocity.x = xBlocked ? 0.0f : playerVelocity.x;
    playerVelocity.y = yBlocked ? 0.0f : playerVelocity.y;
    player->SetVelocity(playerVelocity);
}

void Engine::Run() {
    while (!WindowShouldClose()) {
        if (currentMode == Mode::EDIT) {
            HandleEditModeInput();
        }

        Scene* scene = GetCurrentScene();
        Rectangle editModeCameraArea = GetSceneCameraArea();
        auto& editModeEntities = scene->GetEditModeEntities();
        auto& playModeSnapshots = scene->GetPlayModeSnapshots();

        if (playModeWindowOpen && currentMode == Mode::PLAY) {
            PlayerEntity* playModePlayer = playerManager.FindPlayerEntity(playModeSnapshots);
            float deltaTime = GetFrameTime();
            for (auto& entity : playModeSnapshots) {
                if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
                    player->Update(GetFrameTime());
                }
                else if (auto enemy = dynamic_cast<EnemyEntity*>(entity.get())) {
                    enemy->Update(GetFrameTime(), playModePlayer);
                }
                else {
                    entity->Update(deltaTime);
                }
            }
            ResolveCollisionInPlayMode();
            UpdatePlayModeCamera();
            HandleSceneSwitchInPlayMode();
        }

        testAnimator.Update(GetFrameTime());

        // Test directional animations with number keys
        if (IsKeyPressed(KEY_S)) testAnimator.PlayDirectional("idle", AnimationDirection::DOWN);
        if (IsKeyPressed(KEY_D)) testAnimator.PlayDirectional("idle", AnimationDirection::RIGHT);
        if (IsKeyPressed(KEY_W)) testAnimator.PlayDirectional("idle", AnimationDirection::UP);
        if (IsKeyPressed(KEY_A)) testAnimator.PlayDirectional("idle", AnimationDirection::LEFT);

        if (IsKeyPressed(KEY_SEVEN)) {
            animationEditor.SetOpen(!animationEditor.IsOpen());
        }

        BeginDrawing();
        ClearBackground(GRAY);
        grid.Draw();

        // Draw edit mode
        if (currentMode != Mode::PLAY){
            BeginMode2D(grid.GetCamera());
            DrawEditModeTiles();
            DrawHoveredTileLayerInfo();

            if (editModeCameraArea.width > 0 && editModeCameraArea.height > 0) {
                DrawRectangleLinesEx(editModeCameraArea, 2.0f, GREEN);
                DrawRectangleRec(editModeCameraArea, Fade(GREEN, 0.1f));
            }
            for (auto& entity : editModeEntities) {
                bool shouldDraw = true;

                if (auto* staticEntity = dynamic_cast<StaticEntity*>(entity.get())) {
                    int entityLayer = staticEntity->GetLayer();
                    if (entityLayer >= 0 && entityLayer < tileToolState.totalLayers) {
                        shouldDraw = layerVisibility[entityLayer];
                    }
                }
                // TODO: Add similar checks for PlayerEntity and EnemyEntity
                /*else if (auto* playerEntity = dynamic_cast<PlayerEntity*>(entity.get())) {
                    shouldDraw = layerVisibility[playerEntity->GetLayer()];
                }
                else if (auto* enemyEntity = dynamic_cast<EnemyEntity*>(entity.get())) {
                    shouldDraw = layerVisibility[enemyEntity->GetLayer()];
                }*/
                if (shouldDraw) {
                    entity->Draw();
                }
            }

            DrawAssetPlacementPreview();

            if (testAnimator.HasValidTexture()) {
                Rectangle srcRect = testAnimator.GetCurrentFrameRect();
                Rectangle destRect = {
                    testAnimatorPosition.x,
                    testAnimatorPosition.y,
                    static_cast<float>(testAnimationSet.frameWidth),
                    static_cast<float>(testAnimationSet.frameHeight)
                };
                DrawTexturePro(testAnimator.GetTexture(), srcRect, destRect, {0, 0}, 0.0f, WHITE);
            }

            EndMode2D();
        }

        // Draw play mode
        if (playModeWindowOpen) {
            BeginTextureMode(playModeTexture);
            ClearBackground(BLACK);
            BeginMode2D(playerCamera);
            DrawPlayModeTiles();
            for (auto& entity : playModeSnapshots) {
                bool shouldDraw = true;

                if (auto* staticEntity = dynamic_cast<StaticEntity*>(entity.get())) {
                    int entityLayer = staticEntity->GetLayer();
                    if (entityLayer >= 0 && entityLayer < tileToolState.totalLayers) {
                        shouldDraw = layerVisibility[entityLayer];
                    }
                }

                if (shouldDraw) {
                    entity->Draw();
                }
            }
            EndMode2D();
            EndTextureMode();
        }

        rlImGuiBegin();
        UI::RenderControlPanel(*this, grid);
        UI::RenderDebugConsole();
        UI::RenderAssetConsole(*this);
        UI::RenderPlayModeWindow(*this);
        UI::RenderNewSceneTab(*this);
        if (animationEditor.IsOpen()) {
            animationEditor.Render(*this);
        }
        assetImporter.Render(*this);
        rlImGuiEnd();

        EndDrawing();
    }
}

void Engine::Shutdown() const {
    AssetPersistence::SaveAssetDefinitions(assetManager, "assets.json");
    if (playModeTexture.id != 0) {
        UnloadRenderTexture(playModeTexture);
    }
    CloseWindow();
    rlImGuiShutdown();
}