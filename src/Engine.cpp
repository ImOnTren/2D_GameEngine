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
// =======================================
// =      Camera Selection Logic        =
// =======================================
void Engine::UpdatePlayerCamera() {
    if (!player) {
        playerCameraArea = {0, 0, 0, 0};
        return;
    }

    // Get current resolution
    int width = availableResolutions[selectedResolutionIndex].width;
    int height = availableResolutions[selectedResolutionIndex].height;

    Vector2 playerPos = player->GetPosition();
    Vector2 playerSize = player->GetSize();

    // Calculate camera area (centered on player)
    Vector2 playerCenter = {
        playerPos.x + playerSize.x / 2.0f,
        playerPos.y + playerSize.y / 2.0f
    };

    playerCameraArea = {
        playerCenter.x - width / 2.0f,
        playerCenter.y - height / 2.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };

    // CORRECTED: Set up camera to view the camera area
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
    if (playerManager.TryPlacePlayer(mouseScreen, grid.GetCamera(), player)) {
        UpdatePlayerCamera(); // Update camera when player is placed
    }
}

void Engine::HandlePlayerRemoval() {
    if (IsMouseOverUI()) return;
    Vector2 mouseScreen = GetMousePosition();
    if (playerManager.TryRemovePlayer(mouseScreen, grid.GetCamera(), player)) {
        UpdatePlayerCamera(); // Update camera when player is removed
    }
}
// =======================================
// =           Enemy Functions           =
// =======================================
void Engine::HandleEnemyPlacement() {
    if (IsMouseOverUI()) return;
    Vector2 mouseScreen = GetMousePosition();
    enemyManager.TryPlaceEnemy(mouseScreen, grid.GetCamera(), enemies);
}
void Engine::HandleEnemyRemoval() {
    if (IsMouseOverUI()) return;
    Vector2 mouseScreen = GetMousePosition();
    enemyManager.TryRemoveEnemy(mouseScreen, grid.GetCamera(), enemies);
}
// =======================================
void Engine::Run() {
    while (!WindowShouldClose()) {
        HandleEditModeInput();

        // Update play mode if window is open
        if (playModeWindowOpen && player) {
            // Update player and enemies
            player->Update(GetFrameTime());
            for (auto& enemy : enemies) {
                enemy->Update(GetFrameTime(), player.get());
            }

            // Update camera to follow player
            UpdatePlayerCamera();
        }

        BeginDrawing();
        ClearBackground(GRAY);

        grid.Draw();

        // In edit mode, draw everything with the grid camera
        BeginMode2D(grid.GetCamera());

        // Draw player camera area outline if player exists
        if (player && playerCameraArea.width > 0 && playerCameraArea.height > 0) {
            DrawRectangleLinesEx(playerCameraArea, 2.0f, GREEN);

            // Optional: Draw semi-transparent fill
            DrawRectangleRec(playerCameraArea, Fade(GREEN, 0.1f));
        }

        if (player) {
            player->Draw();
        }
        for (auto& enemy : enemies) {
            enemy->Draw();
        }
        EndMode2D();

        // Render play mode to texture if window is open
        if (playModeWindowOpen && player) {
            BeginTextureMode(playModeTexture);
            ClearBackground(BLACK);

            BeginMode2D(playerCamera);

            // Draw the camera area background (optional)
            DrawRectangleRec(playerCameraArea, DARKGRAY);

            // Draw entities
            if (player) {
                player->Draw();
            }
            for (auto& enemy : enemies) {
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