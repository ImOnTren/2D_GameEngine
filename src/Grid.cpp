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

void Grid::Update() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float panelWidth = static_cast<float>(screenWidth) / 4.0f;
    GridCamera.offset = { panelWidth, 0 };

    if (!initialized) {
        gridTexture = LoadRenderTexture(screenWidth, screenHeight);

        // Pixel-perfect position for the grid (avoid floating pixels)
        SetTextureFilter(gridTexture.texture, TEXTURE_FILTER_POINT);

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
    int gridTileSize = GetTileSize();

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const float panelWidth = static_cast<float>(screenWidth) / 4.0f;
    const float panelHeight = static_cast<float>(screenHeight) - (static_cast<float>(screenHeight) / 4.0f);
    const float sceneTabHeight = static_cast<float>(screenHeight) / 18.0f;

    int startX, endX, startY, endY;

    if (needsRedraw) {
        BeginTextureMode(gridTexture);
        ClearBackground(GRAY);

        BeginMode2D(GridCamera);

        Vector2 topLeft = GetScreenToWorld2D({ panelWidth, sceneTabHeight }, GridCamera);
        Vector2 bottomRight = GetScreenToWorld2D({ static_cast<float>(screenWidth), panelHeight }, GridCamera);

        startX = static_cast<int>(std::ceil(topLeft.x / static_cast<float>(gridTileSize)));
        endX = static_cast<int>(bottomRight.x / static_cast<float>(gridTileSize));
        startY = static_cast<int>(std::ceil(topLeft.y / static_cast<float>(gridTileSize)));
        endY = static_cast<int>(bottomRight.y / static_cast<float>(gridTileSize));

        if (startX < 0) startX = 0;
        if (startY < 0) startY = 0;
        //==================
        // Draw grid lines
        for (int y = startY; y <= endY; y++) {
            for (int x = startX; x <= endX; x++) {
                DrawRectangleLines(x * gridTileSize, y * gridTileSize, gridTileSize, gridTileSize, DARKGRAY);
            }
        }
        //==================
        EndMode2D();
        EndTextureMode();

        needsRedraw = false;
    } else {
        // Calculate grid bounds for highlight check
        Vector2 topLeft = GetScreenToWorld2D({ panelWidth, sceneTabHeight }, GridCamera);
        Vector2 bottomRight = GetScreenToWorld2D({ static_cast<float>(screenWidth), static_cast<float>(screenHeight) }, GridCamera);

        startX = static_cast<int>(std::ceil(topLeft.x / static_cast<float>(gridTileSize)));
        endX = static_cast<int>(bottomRight.x / static_cast<float>(gridTileSize));
        startY = static_cast<int>(std::ceil(topLeft.y / static_cast<float>(gridTileSize)));
        endY = static_cast<int>(bottomRight.y / static_cast<float>(gridTileSize));

        if (startX < 0) startX = 0;
        if (startY < 0) startY = 0;
    }

    const Rectangle sourceRect = {
        0.0f,
        0.0f,
        static_cast<float>(gridTexture.texture.width),
        -static_cast<float>(gridTexture.texture.height)
    };

    Vector2 position = { 0.0f, 0.0f };

    DrawTextureRec(gridTexture.texture, sourceRect, position, WHITE);

    // --- Highlight hovered cell only if inside grid bounds ---
    Vector2 mouseScreen = GetMousePosition();
    if (mouseScreen.x > panelWidth && mouseScreen.y < panelHeight) {
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, GridCamera);
        const int cellX = static_cast<int>(mouseWorld.x / static_cast<float>(gridTileSize));
        const int cellY = static_cast<int>(mouseWorld.y / static_cast<float>(gridTileSize));

        if (cellX >= startX && cellX <= endX && cellY >= startY && cellY <= endY) {
            const int cellScreenX = cellX * gridTileSize;
            const int cellScreenY = cellY * gridTileSize;

            BeginMode2D(GridCamera);
            DrawRectangle(cellScreenX, cellScreenY, gridTileSize, gridTileSize, highlightColor);
            EndMode2D();
        }
    }
}
