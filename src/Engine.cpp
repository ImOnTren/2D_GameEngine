#include "Engine.h"

using namespace std;

Engine::Engine() : playerManager(grid), enemyManager(grid) {
    // Set default camera resolution (640x360)
    playerCamera.offset = {0, 0};
    playerCamera.target = {0, 0};
    playerCamera.rotation = 0.0f;
    playerCamera.zoom = 1.0f;
    playModeTexture = {0};
    playModeWindowOpen = false;

    // Initialize player camera area with default resolution
    UpdatePlayerCamera();
}

Engine::~Engine() { }

void Engine::Init() {
    InitWindow(800, 400, "Game Engine - Edit Mode");
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(60);
    rlImGuiSetup(true);

    // Initialize play mode render texture with default resolution
    playModeTexture = LoadRenderTexture(availableResolutions[selectedResolutionIndex].width,
                                       availableResolutions[selectedResolutionIndex].height);
}

bool Engine::IsMouseOverUI() const {
    // Check if mouse is over any ImGui window
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
        case ToolState::NONE:
            // Allow grid movement only when no tool is active
            grid.Update();
            break;
    }
}

void Engine::StartPlayMode() {
    PlayerEntity* player = FindPlayerEntity();
    if (player) {
        CreatePlayModeSnapshots(); // Create lightweight snapshots
        playModeWindowOpen = true;
        currentMode = Mode::PLAY;
        UpdatePlayModeCamera();
    } else {
        playModeWindowOpen = false;
        currentMode = Mode::EDIT;
        TraceLog(LOG_WARNING, "Cannot start play mode: No player placed");
    }
}

void Engine::StopPlayMode() {
    playModeWindowOpen = false;
    currentMode = Mode::EDIT;
    // Clear play mode entities
    playModeSnapshots.clear();
}

void Engine::CreatePlayModeSnapshots() {
    playModeSnapshots.clear();
    for (auto& entity : editModeEntities) {
        playModeSnapshots.push_back(entity->CreateSnapshot());
    }
}

void Engine::RestoreFromSnapshots() {
    for (size_t i = 0; i < editModeEntities.size() && i < playModeSnapshots.size(); ++i) {
        editModeEntities[i]->RestoreFromSnapshot(playModeSnapshots[i].get());
    }
}

// =======================================
// =            Camera Logic             =
// =======================================
void Engine::UpdateEditModeCamera() {
    PlayerEntity* player = FindPlayerEntity();
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
    // Find player in play mode snapshots
    PlayerEntity* player = nullptr;
    for (auto& entity : playModeSnapshots) {
        if (auto p = dynamic_cast<PlayerEntity*>(entity.get())) {
            player = p;
            break;
        }
    }

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
// =          Player Functions           =
// =======================================
PlayerEntity* Engine::FindPlayerEntity() {
    for (auto& entity : editModeEntities) {
        if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
            return player;
        }
    }
    return nullptr;
}

void Engine::HandlePlayerPlacement() {
    if (IsMouseOverUI()) return;

    if (FindPlayerEntity()) {
        TraceLog(LOG_WARNING, "Player already exists. Remove current player first.");
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        std::unique_ptr<PlayerEntity> tempPlayer;
        if (playerManager.TryPlacePlayer(mouseScreen, grid.GetCamera(), tempPlayer)) {
            if (tempPlayer) {
                editModeEntities.push_back(std::move(tempPlayer));
                UpdateEditModeCamera();
            }
        }
    }
}

void Engine::HandlePlayerRemoval() {
    if (IsMouseOverUI()) return;

    // Only remove on mouse click
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, grid.GetCamera());

        for (auto it = editModeEntities.begin(); it != editModeEntities.end(); ++it) {
            if (auto player = dynamic_cast<PlayerEntity*>(it->get())) {
                Rectangle playerBounds = player->GetBounds();

                // Check if mouse is inside player bounds
                if (CheckCollisionPointRec(mouseWorld, playerBounds)) {
                    editModeEntities.erase(it);
                    UpdateEditModeCamera();
                    return; // Exit after removing one player
                }
            }
        }
    }
}
// =======================================
// =           Enemy Functions           =
// =======================================
void Engine::HandleEnemyPlacement() {
    if (IsMouseOverUI()) return;

    // Only place on mouse click
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();

        // Count current enemies
        int enemyCount = 0;
        for (auto& entity : editModeEntities) {
            if (dynamic_cast<EnemyEntity*>(entity.get())) {
                enemyCount++;
            }
        }

        // Enforce enemy limit
        if (enemyCount >= 10) {
            TraceLog(LOG_WARNING, "Maximum number of enemies reached (10). Cannot place more.");
            return;
        }

        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, grid.GetCamera());
        int tileSize = grid.GetTileSize();
        int gridX = static_cast<int>(mouseWorld.x / tileSize);
        int gridY = static_cast<int>(mouseWorld.y / tileSize);

        auto newEnemy = std::make_unique<EnemyEntity>(grid, gridX, gridY);
        editModeEntities.push_back(std::move(newEnemy));
    }
}
void Engine::HandleEnemyRemoval() {
    if (IsMouseOverUI()) return;

    // Only remove on mouse click
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseScreen = GetMousePosition();
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, grid.GetCamera());

        for (auto it = editModeEntities.begin(); it != editModeEntities.end(); ++it) {
            if (auto enemy = dynamic_cast<EnemyEntity*>(it->get())) {
                Rectangle enemyBounds = enemy->GetBounds();

                // Check if mouse is inside enemy bounds
                if (CheckCollisionPointRec(mouseWorld, enemyBounds)) {
                    editModeEntities.erase(it);
                    return; // Exit after removing one enemy
                }
            }
        }
    }
}

int Engine::GetEnemyCount() const {
    int count = 0;
    for (auto& entity : editModeEntities) {
        if (dynamic_cast<EnemyEntity*>(entity.get())) {
            count++;
        }
    }
    return count;
}

// =======================================
void Engine::Run() {
    while (!WindowShouldClose()) {
        if (currentMode == Mode::EDIT) {
            HandleEditModeInput();
        }

        if (playModeWindowOpen && currentMode == Mode::PLAY) {
            // Find player in play mode for enemy targeting
            PlayerEntity* playModePlayer = nullptr;
            for (auto& entity : playModeSnapshots) {
                if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
                    playModePlayer = player;
                    break;
                }
            }

            // Update play mode entities
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
            DrawRectangleRec(playModeCameraArea, DARKGRAY);
            for (auto& entity : playModeSnapshots) {
                entity->Draw();
            }
            EndMode2D();
            EndTextureMode();
        }

        rlImGuiBegin();
        UI::RenderControlPanel(*this, grid);
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