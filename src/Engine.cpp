#include "Engine.h"

using namespace std;

Engine::Engine() : playerManager(grid), enemyManager(grid) {
    playerCamera.offset = {0, 0};
    playerCamera.target = {0, 0};
    playerCamera.rotation = 0.0f;
    playerCamera.zoom = 1.0f;
    playModeTexture = {0};
    playModeWindowOpen = false;
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
        case ToolState::NONE:
            grid.Update();
            break;
    }
}

void Engine::StartPlayMode() {
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
        UI::SetDebugMessage("[WARNING] Cannot start play mode: No player placed");
    }
}

void Engine::StopPlayMode() {
    playModeWindowOpen = false;
    currentMode = Mode::EDIT;
    playModeSnapshots.clear();
    playModeTileMap.reset();
}

void Engine::CreatePlayModeSnapshots() {
    playModeSnapshots.clear();
    for (auto& entity : editModeEntities) {
        playModeSnapshots.push_back(entity->CreateSnapshot());
    }
}

// =======================================
// =          Input Handlers            =
// =======================================
void Engine::HandlePlayerPlacement() {
    if (IsMouseOverUI()) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        playerManager.PlacePlayer(mouseScreen, grid.GetCamera(), editModeEntities);
        UpdateEditModeCamera();
    }
}

void Engine::HandlePlayerRemoval() {
    if (IsMouseOverUI()) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        if (playerManager.RemovePlayer(mouseScreen, grid.GetCamera(), editModeEntities)) {
            UpdateEditModeCamera();
        }
    }
}

void Engine::HandleEnemyPlacement() {
    if (IsMouseOverUI()) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        enemyManager.PlaceEnemy(mouseScreen, grid.GetCamera(), editModeEntities);
    }
}

void Engine::HandleEnemyRemoval() {
    if (IsMouseOverUI()) return;

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

    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(worldPos.x / tileSize);
    int gridY = static_cast<int>(worldPos.y / tileSize);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        TileData tile;
        tile.tileID = tileToolState.tileset->id;
        tile.tileIndex = tileToolState.selectedTileIndex;

        tileMap.SetTile(gridX, gridY, tile);

        UI::SetDebugMessage("[INFO] Tile placed at (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")");
    }
}

void Engine::HandleTileRemoval()
{
    if (IsMouseOverUI()) return;

    Vector2 mouseScreen = GetMousePosition();
    Vector2 worldPos = GetScreenToWorld2D(mouseScreen, grid.GetCamera());

    int tileSize = grid.GetTileSize();
    int GridX = static_cast<int>(worldPos.x / tileSize);
    int GridY = static_cast<int>(worldPos.y / tileSize);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (tileMap.HasTile(GridX, GridY))
        {
            tileMap.RemoveTile(GridX, GridY);
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

    for (const auto& [key, tileData] : tileMap.GetAllTiles()) {
        int x = static_cast<int>(key >> 32);
        int y = static_cast<int>(key & 0xFFFFFFFF);

        Asset* asset = assetManager.GetAsset(tileData.tileID);
        if (!asset || !asset->loaded) continue;

        Rectangle sourceRect = assetManager.GetSpecificSprite(asset, tileData.tileIndex);
        Rectangle destRect = {
            static_cast<float>(x * tileSize),
            static_cast<float>(y * tileSize),
            static_cast<float>(tileSize),
            static_cast<float>(tileSize)
        };

        DrawTexturePro(asset->texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
}

void Engine::DrawPlayModeTiles()
{
    if (!playModeTileMap) return;

    int tileSize = grid.GetTileSize();

    for (const auto& [key, tileData] : playModeTileMap->GetAllTiles()) {
        int x = static_cast<int>(key >> 32);
        int y = static_cast<int>(key & 0xFFFFFFFF);

        Asset* asset = assetManager.GetAsset(tileData.tileID);
        if (!asset || !asset->loaded) continue;

        Rectangle sourceRect = assetManager.GetSpecificSprite(asset, tileData.tileIndex);
        Rectangle destRect = {
            static_cast<float>(x * tileSize),
            static_cast<float>(y * tileSize),
            static_cast<float>(tileSize),
            static_cast<float>(tileSize)
        };

        DrawTexturePro(asset->texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
}

// =======================================
// =            Camera Logic            =
// =======================================
void Engine::UpdateEditModeCamera() {
    PlayerEntity* player = playerManager.FindPlayerEntity(editModeEntities);
    if (!player) {
        editModeCameraArea = {0, 0, 0, 0};
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

    editModeCameraArea = {
        playerCenter.x - width / 2.0f,
        playerCenter.y - height / 2.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };
}

void Engine::UpdatePlayerCamera() {
    if (currentMode == Mode::PLAY) {
        UpdatePlayModeCamera();
    } else {
        UpdateEditModeCamera();
    }
}

void Engine::UpdatePlayModeCamera() {
    PlayerEntity* player = playerManager.FindPlayerEntity(playModeSnapshots);
    if (!player) {
        playModeCameraArea = {0, 0, 0, 0};
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

    playModeCameraArea = {
        playerCenter.x - width / 2.0f,
        playerCenter.y - height / 2.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };

    playerCamera.target = playerCenter;
    playerCamera.offset = {width / 2.0f, height / 2.0f};
    playerCamera.rotation = 0.0f;
    playerCamera.zoom = 1.0f;

    if (playModeTexture.texture.width != width || playModeTexture.texture.height != height) {
        if (playModeTexture.id != 0) {
            UnloadRenderTexture(playModeTexture);
        }
        playModeTexture = LoadRenderTexture(width, height);
    }
}

// =======================================
bool Engine::HasPlayer() {
    return playerManager.PlayerExists(editModeEntities);
}

int Engine::GetEnemyCount() const {
    return enemyManager.GetEnemyCount(editModeEntities);
}

void Engine::Run() {
    while (!WindowShouldClose()) {
        if (currentMode == Mode::EDIT) {
            HandleEditModeInput();
        }

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
            UpdatePlayModeCamera();
        }

        BeginDrawing();
        ClearBackground(GRAY);
        grid.Draw();

        // Draw edit mode
        BeginMode2D(grid.GetCamera());
        DrawEditModeTiles();
        if (editModeCameraArea.width > 0 && editModeCameraArea.height > 0) {
            DrawRectangleLinesEx(editModeCameraArea, 2.0f, GREEN);
            DrawRectangleRec(editModeCameraArea, Fade(GREEN, 0.1f));
        }
        for (auto& entity : editModeEntities) {
            entity->Draw();
        }
        EndMode2D();

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
        rlImGuiEnd();

        EndDrawing();
    }
}

void Engine::Shutdown() {
    if (playModeTexture.id != 0) {
        UnloadRenderTexture(playModeTexture);
    }
    CloseWindow();
    rlImGuiShutdown();
}