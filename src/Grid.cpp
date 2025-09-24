#include "Grid.h"

Grid::Grid() {
    GridCamera.offset = { 0, 0 };
    GridCamera.target = { 0, 0 };
    GridCamera.rotation = 0.0f;
    GridCamera.zoom = zoom;
}

Grid::~Grid() {
    if (initialized) UnloadRenderTexture(gridTexture);
}

void Grid::RenderSizeAvailability() {
    for (int i = 0; i < tileSize.size(); i++) {
        ImGui::SameLine();
        if (ImGui::RadioButton(tileSize[i].c_str(), selectedTileSizeIndex == i)) {
            selectedTileSizeIndex = i;
            needsRedraw = true;
        }
    }
}

void Grid::Update() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float panelWidth = screenWidth / 4.0f;
    GridCamera.offset = { panelWidth, 0 };

    if (!initialized) {
        gridTexture = LoadRenderTexture(screenWidth, screenHeight);
        initialized = true;
        needsRedraw = true;
    }
    // zooming with mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 mouseScreen = GetMousePosition();
        Vector2 worldBeforeZoom = GetScreenToWorld2D(mouseScreen, GridCamera);

        zoom += wheel * 0.1f;
        if (zoom < 0.1f) zoom = 0.1f;
        if (zoom > 4.0f) zoom = 4.0f;
        GridCamera.zoom = zoom;

        Vector2 worldAfterZoom = GetScreenToWorld2D(mouseScreen, GridCamera);
        Vector2 offset = Vector2Subtract(worldBeforeZoom, worldAfterZoom);
        GridCamera.target = Vector2Add(GridCamera.target, offset);

        needsRedraw = true;
    }
    // movement with left mouse button
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f / GridCamera.zoom);
        GridCamera.target = Vector2Add(GridCamera.target, delta);
        needsRedraw = true;
    }
}

void Grid::Draw() {
    int tileSize = GetTileSize();

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float panelWidth = screenWidth / 4.0f;

    int startX, endX, startY, endY;

    if (needsRedraw) {
        BeginTextureMode(gridTexture);
        ClearBackground(GRAY);

        BeginMode2D(GridCamera);

        Vector2 topLeft = GetScreenToWorld2D({ panelWidth, 0 }, GridCamera);
        Vector2 bottomRight = GetScreenToWorld2D({ (float)screenWidth, (float)screenHeight }, GridCamera);

        startX = (int)std::ceil(topLeft.x / tileSize);
        endX = (int)(bottomRight.x / tileSize);
        startY = (int)(topLeft.y / tileSize);
        endY = (int)(bottomRight.y / tileSize);

        if (startX < 0) startX = 0;
        if (startY < 0) startY = 0;
        //==================
        // Draw grid lines
        for (int y = startY; y <= endY; y++) {
            for (int x = startX; x <= endX; x++) {
                DrawRectangleLines(x * tileSize, y * tileSize, tileSize, tileSize, DARKGRAY);
            }
        }
        //==================
        EndMode2D();
        EndTextureMode();

        needsRedraw = false;
    } else {
        // Calculate grid bounds for highlight check
        Vector2 topLeft = GetScreenToWorld2D({ panelWidth, 0 }, GridCamera);
        Vector2 bottomRight = GetScreenToWorld2D({ (float)screenWidth, (float)screenHeight }, GridCamera);

        startX = (int)std::ceil(topLeft.x / tileSize);
        endX = (int)(bottomRight.x / tileSize);
        startY = (int)(topLeft.y / tileSize);
        endY = (int)(bottomRight.y / tileSize);

        if (startX < 0) startX = 0;
        if (startY < 0) startY = 0;
    }

    Rectangle sourceRect = {
        0.0f,
        0.0f,
        (float)gridTexture.texture.width,
        -(float)gridTexture.texture.height
    };

    Vector2 position = { 0.0f, 0.0f };

    DrawTextureRec(gridTexture.texture, sourceRect, position, WHITE);

    // --- Highlight hovered cell only if inside grid bounds ---
    Vector2 mouseScreen = GetMousePosition();
    if (mouseScreen.x > panelWidth) {
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, GridCamera);
        int cellX = (int)(mouseWorld.x / tileSize);
        int cellY = (int)(mouseWorld.y / tileSize);

        if (cellX >= startX && cellX <= endX && cellY >= startY && cellY <= endY) {
            int cellScreenX = cellX * tileSize;
            int cellScreenY = cellY * tileSize;

            BeginMode2D(GridCamera);
            DrawRectangle(cellScreenX, cellScreenY, tileSize, tileSize, highlightColor);
            EndMode2D();
        }
    }
}