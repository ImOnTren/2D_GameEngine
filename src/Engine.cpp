#include "Engine.h"

using namespace std;

Engine::Engine() : playerManager(grid), enemyManager(grid) {
    playerCamera.offset = {0, 0};
    playerCamera.target = {0, 0};
    playerCamera.rotation = 0.0f;
    playerCamera.zoom = 1.0f;
    playModeTexture = {0};
    playModeWindowOpen = false;
    HandleSceneCreation();
    UpdatePlayerCamera();
}

Engine::~Engine(){
    assetManager.UnloadAllAssets();
}

void Engine::Init() {
    InitWindow(800, 400, "Game Engine - Edit Mode");
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(144);
    rlImGuiSetup(true);

    playModeTexture = LoadRenderTexture(availableResolutions[selectedResolutionIndex].width,
                                       availableResolutions[selectedResolutionIndex].height);

    LoadAssets();
}

void Engine::LoadAssets()
{
    assetManager.LoadAsset("player_hugo", "Hugo_sprite", "Player", "../src/player/hugo.png");
    assetManager.LoadAsset("grass_tileset", "Grass_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Tileset Grass Spring.png", 16, 16);
    assetManager.LoadAsset("winter_tileset", "Winter_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Tileset Grass Winter.png", 16, 16);
    assetManager.LoadAsset("water_tileset", "Water_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Water Ground animations tiles.png", 16, 16);
    assetManager.LoadAsset("caves_tileset", "Caves_tileset", "Tileset", "../src/assets/Farm/Tileset/Modular/Caves.png", 16, 16);
    assetManager.LoadAsset("broken_house1", "Broken_House_1_texture", "Static Texture", "../src/assets/Exterior/Houses/4.png");
}


bool Engine::IsMouseOverUI() const {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

void Engine::HandleEditModeInput() {
    if (currentMode != Mode::EDIT || IsMouseOverUI()) return;

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
            HandleAssetPlacement();
            break;
        case ToolState::NONE:
            grid.Update();
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
    if (IsMouseOverUI()) return;

    Vector2 mouseScreen = GetMousePosition();
    Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());
    Scene* scene = GetCurrentScene();
    TileMap& tileMap = scene->GetTileMap();

    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(worldPos.x) / tileSize;
    int gridY = static_cast<int>(worldPos.y) / tileSize;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        TileData tile;
        tile.tileID = tileToolState.tileset->id;
        tile.tileIndex = tileToolState.selectedTileIndex;

        tileMap.SetTile(gridX, gridY, tile, tileToolState.activeLayer);

        UI::SetDebugMessage("[INFO] Tile placed at (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")");
    }
}

void Engine::HandleTileRemoval()
{
    if (IsMouseOverUI()) return;

    Vector2 mouseScreen = GetMousePosition();
    Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());
    Scene* scene = GetCurrentScene();
    TileMap& tileMap = scene->GetTileMap();

    int tileSize = grid.GetTileSize();
    int GridX = static_cast<int>(worldPos.x) / tileSize;
    int GridY = static_cast<int>(worldPos.y) / tileSize;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (tileMap.HasTile(GridX, GridY))
        {
            tileMap.RemoveTileFromLayer(GridX, GridY, tileToolState.activeLayer);
            UI::SetDebugMessage("[INFO] Tile removed at (" + std::to_string(GridX) + ", " + std::to_string(GridY) + ")");
        }
        else
        {
            UI::SetDebugMessage("[INFO] No tile to remove at (" + std::to_string(GridX) + ", " + std::to_string(GridY) + ")");
        }
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

        for (const auto& tileData : tilesAtPos) {
            Asset* asset = assetManager.GetAsset(tileData.tileID);
            if (!asset || !asset->loaded) continue;

            const Rectangle sourceRect = assetManager.GetSpecificSprite(asset, tileData.tileIndex);
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

    Scene* scene = GetCurrentScene();
    if (!scene) return;

    const TileMap& tileMap = scene->GetTileMap();
    const std::vector<TileData> tilesAtPos = tileMap.GetTilesAtPosition(gridX, gridY);

    if (tilesAtPos.empty()) return;

    // Layer colors
    Color layerColors[5] = {
        {50, 100, 200, 200},   // Layer 0: Blue (Background)
        {50, 200, 50, 200},    // Layer 1: Green (Ground)
        {200, 150, 50, 200},   // Layer 2: Orange (Decoration)
        {200, 50, 150, 200},   // Layer 3: Pink (Foreground)
        {150, 50, 200, 200}    // Layer 4: Purple (Overlay)
    };

    // Draw colored squares in the corner of the hovered cell
    const float cellX = static_cast<float>(gridX * tileSize);
    const float cellY = static_cast<float>(gridY * tileSize);
    float indicatorSize = static_cast<float>(tileSize) / 5.0f;
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

void Engine::SetCurrentTileLayer(int layer) {
    tileToolState.activeLayer = layer;
}

int Engine::GetCurrentTileLayer() const {
    return tileToolState.activeLayer;
}

void Engine::CycleLayerUp() {
    tileToolState.activeLayer--;
}

void Engine::CycleLayerDown() {
    tileToolState.activeLayer++;
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

        auto *newAsset = new StaticEntity(grid, assetToolState.asset,
                                      static_cast<int>(worldPos.x) / grid.GetTileSize(),
                                      static_cast<int>(worldPos.y) / grid.GetTileSize());
        scene->GetEditModeEntities().emplace_back(newAsset);
        UI::SetDebugMessage("[INFO] Placed asset '" + assetToolState.asset->name + "' at (" +
                            std::to_string(static_cast<int>(worldPos.x) / grid.GetTileSize()) + ", " +
                            std::to_string(static_cast<int>(worldPos.y) / grid.GetTileSize()) + ")");
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

    TileData tile = playModeTileMap->GetTile(playerGridX, playerGridY);
    if (!tile.isSceneSwitcher) {
        return;
    }
    if (tile.triggerKey == 0) {
        return;
    }
    if (IsKeyPressed(tile.triggerKey)) {
        if (tile.targetSceneID.empty()) {
            UI::SetDebugMessage("[ERROR] Tile at (" + std::to_string(playerGridX) + ", " + std::to_string(playerGridY) + ") has no target scene ID set");
            return;
        }
        ChangeScene(tile.targetSceneID, playerGridX, playerGridY);
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

    if (dx == 0.0f && dy == 0.0f) {
        return;
    }

    Rectangle playerBounds = player->GetBounds();
    float width = playerBounds.width;
    float height = playerBounds.height;

    int tileSize = grid.GetTileSize();

    auto collidesAt = [&](float x, float y) -> bool {
        Rectangle r{x, y, width, height};
        int startX = static_cast<int>(r.x) / tileSize;
        int startY = static_cast<int>(r.y) / tileSize;
        int endX = static_cast<int>(r.x + r.width - 1) / tileSize;
        int endY = static_cast<int>(r.y + r.height - 1) / tileSize;
        for (int ty = startY; ty <= endY; ++ty) {
            for (int tx = startX; tx <= endX; ++tx) {
                if (!playModeTileMap->HasTile(tx, ty)) {
                    continue;
                }
                TileData tile = playModeTileMap->GetTile(tx, ty);
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
        for (auto& entity : playModeSnapshots) {
            if (entity.get() == player) {
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

    Vector2 resolvedPos = previousPlayerPos;
    bool xBlocked = false;
    bool yBlocked = false;
    if (dx != 0.0f) {
        float candX = previousPlayerPos.x + dx;
        if (!collidesAt(candX, previousPlayerPos.y)) {
            resolvedPos.x = candX;
        }
        else {
            xBlocked = true;
        }
    }
    if (dy != 0.0f) {
        float candY = previousPlayerPos.y + dy;
        if (!collidesAt(resolvedPos.x, candY)) {
            resolvedPos.y = candY;
        }
        else {
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

            for (auto& entity : playModeSnapshots) {
                if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
                    player->Update(GetFrameTime());
                }
                else if (auto enemy = dynamic_cast<EnemyEntity*>(entity.get())) {
                    enemy->Update(GetFrameTime(), playModePlayer);
                }
            }
            ResolveCollisionInPlayMode();
            UpdatePlayModeCamera();
            HandleSceneSwitchInPlayMode();
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
                entity->Draw();
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
                entity->Draw();
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
        rlImGuiEnd();

        EndDrawing();
    }
}

void Engine::Shutdown() const {
    if (playModeTexture.id != 0) {
        UnloadRenderTexture(playModeTexture);
    }
    CloseWindow();
    rlImGuiShutdown();
}