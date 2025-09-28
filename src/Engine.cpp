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
    if (editModePlayer) {
        CopyEditToPlayMode();
        playModeWindowOpen = true;
        currentMode = Mode::PLAY;

        UpdatePlayModeCamera();
    } else {
        // Can't start play mode without a player
        playModeWindowOpen = false;
        currentMode = Mode::EDIT;
        TraceLog(LOG_WARNING, "Cannot start play mode: No player placed");
    }
}

void Engine::StopPlayMode() {
    playModeWindowOpen = false;
    currentMode = Mode::EDIT;
    // Clear play mode entities
    playModePlayer.reset();
    playModeEnemies.clear();
}

void Engine::CopyEditToPlayMode() {
    // Copy player
    if (editModePlayer) {
        playModePlayer = std::make_unique<PlayerEntity>(grid, editModePlayer->GetGridX(), editModePlayer->GetGridY());
        playModePlayer->SetPosition(editModePlayer->GetPosition());
    }

    // Copy enemies
    playModeEnemies.clear();
    for (auto& enemy : editModeEnemies) {
        auto newEnemy = std::make_unique<EnemyEntity>(grid, enemy->GetGridX(), enemy->GetGridY());
        newEnemy->SetPosition(enemy->GetPosition());
        playModeEnemies.push_back(std::move(newEnemy));
    }

    // Update camera for play mode
    UpdatePlayerCamera();
}

// =======================================
// =            Camera Logic             =
// =======================================
void Engine::UpdatePlayerCamera() {
    if (currentMode == Mode::PLAY) {
        UpdatePlayModeCamera();
    } else {
        UpdateEditModeCamera();
    }
}

void Engine::UpdateEditModeCamera() {
    if (!editModePlayer) {
        editModeCameraArea = {0, 0, 0, 0};
        return;
    }

    // Get current resolution
    int width = availableResolutions[selectedResolutionIndex].width;
    int height = availableResolutions[selectedResolutionIndex].height;

    Vector2 playerPos = editModePlayer->GetPosition();
    Vector2 playerSize = editModePlayer->GetSize();

    // Calculate camera area (centered on player)
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
void Engine::UpdatePlayModeCamera() {
    if (!playModePlayer) {
        playModeCameraArea = {0, 0, 0, 0};
        return;
    }

    // Get current resolution
    int width = availableResolutions[selectedResolutionIndex].width;
    int height = availableResolutions[selectedResolutionIndex].height;

    Vector2 playerPos = playModePlayer->GetPosition();
    Vector2 playerSize = playModePlayer->GetSize();

    // Calculate camera area (centered on player)
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

    // Set up camera to view the camera area
    playerCamera.target = playerCenter;
    playerCamera.offset = {width / 2.0f, height / 2.0f};
    playerCamera.rotation = 0.0f;
    playerCamera.zoom = 1.0f;

    // Update render texture size if needed
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
void Engine::HandlePlayerPlacement() {
    if (IsMouseOverUI()) return;
    Vector2 mouseScreen = GetMousePosition();
    if (playerManager.TryPlacePlayer(mouseScreen, grid.GetCamera(), editModePlayer)) {
        UpdateEditModeCamera(); // Update camera when player is placed
    }
}

void Engine::HandlePlayerRemoval() {
    if (IsMouseOverUI()) return;
    Vector2 mouseScreen = GetMousePosition();
    if (playerManager.TryRemovePlayer(mouseScreen, grid.GetCamera(), editModePlayer)) {
        UpdateEditModeCamera(); // Update camera when player is removed
    }
}
// =======================================
// =           Enemy Functions           =
// =======================================
void Engine::HandleEnemyPlacement() {
    if (IsMouseOverUI()) return;
    Vector2 mouseScreen = GetMousePosition();
    enemyManager.TryPlaceEnemy(mouseScreen, grid.GetCamera(), editModeEnemies);
}
void Engine::HandleEnemyRemoval() {
    if (IsMouseOverUI()) return;
    Vector2 mouseScreen = GetMousePosition();
    enemyManager.TryRemoveEnemy(mouseScreen, grid.GetCamera(), editModeEnemies);
}
// =======================================
void Engine::Run() {
    while (!WindowShouldClose()) {
        // Handle input based on current mode
        if (currentMode == Mode::EDIT) {
            HandleEditModeInput();
        }

        // Update play mode if window is open
        if (playModeWindowOpen && playModePlayer) {
            // Update play mode player and enemies (separate from edit mode)
            playModePlayer->Update(GetFrameTime());
            for (auto& enemy : playModeEnemies) {
                enemy->Update(GetFrameTime(), playModePlayer.get());
            }

            // Update camera to follow play mode player
            UpdatePlayModeCamera();
        }

        BeginDrawing();
        ClearBackground(GRAY);

        grid.Draw();

        // Always draw edit mode entities in the grid view
        BeginMode2D(grid.GetCamera());

        // Draw edit mode camera area (frozen during play mode)
        if (editModePlayer && editModeCameraArea.width > 0 && editModeCameraArea.height > 0) {
            DrawRectangleLinesEx(editModeCameraArea, 2.0f, GREEN);
            DrawRectangleRec(editModeCameraArea, Fade(GREEN, 0.1f));
        }

        // Draw edit mode entities (always visible in grid view)
        if (editModePlayer) {
            editModePlayer->Draw();
        }
        for (auto& enemy : editModeEnemies) {
            enemy->Draw();
        }
        EndMode2D();

        // Render play mode to texture if window is open
        if (playModeWindowOpen && playModePlayer) {
            BeginTextureMode(playModeTexture);
            ClearBackground(BLACK);

            BeginMode2D(playerCamera);

            // Draw the camera area background (optional)
            DrawRectangleRec(playModeCameraArea, DARKGRAY);

            // Draw play mode entities (separate from edit mode)
            if (playModePlayer) {
                playModePlayer->Draw();
            }
            for (auto& enemy : playModeEnemies) {
                enemy->Draw();
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