#include "PlayerEntity.h"

PlayerEntity::PlayerEntity(Grid& grid, Asset* asset, const int gridX, const int gridY, const int layer)
    : Entity({0,0}, {16,16}, layer),
      grid(grid),
      cellX(gridX),
      cellY(gridY),
      layer(layer),
      asset(asset) {

    float w = static_cast<float>(grid.GetTileSize());
    float h = static_cast<float>(grid.GetTileSize());

    if (asset && asset->loaded) {
        if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
            w = asset->SpriteSourceRect.width;
            h = asset->SpriteSourceRect.height;
        } else {
            w = static_cast<float>(asset->texture.width);
            h = static_cast<float>(asset->texture.height);
        }
    }

    previousPosition = position;
    previousGridX = cellX;
    previousGridY = cellY;

    baseSize = { w, h };
    size = baseSize;
    scale = 1.0f;

    PlaceOnGrid(gridX, gridY);
    RecalculateCollisionBox();
}

PlayerEntity::PlayerEntity(const PlayerEntity& other)
    : Entity(other),
      grid(other.grid),
      cellX(other.cellX),
      cellY(other.cellY),
      layer(other.layer),
      asset(other.asset),
      speed(other.speed),
      playerTexture(other.playerTexture),
      textureLoaded(other.textureLoaded),
      scale(other.scale),
      baseSize(other.baseSize) {
}

PlayerEntity::~PlayerEntity() {
    if (textureLoaded) {
        UnloadTexture(playerTexture);
        textureLoaded = false;
    }
}

void PlayerEntity::RecalculateCollisionBox() {
    float hitboxWidth  = size.x * 0.45f;
    float hitboxHeight = size.y * 0.75f;
    float hitboxOffsetX = (size.x - hitboxWidth) * 0.5f;
    float hitboxOffsetY = size.y - hitboxHeight - (4.0f * scale);

    SetCollisionBox({ hitboxOffsetX, hitboxOffsetY }, { hitboxWidth, hitboxHeight });
}

void PlayerEntity::SetScale(float newScale) {
    if (newScale < 0.25f) newScale = 0.25f;
    if (newScale > 4.0f)  newScale = 4.0f;

    scale = newScale;
    size = { baseSize.x * scale, baseSize.y * scale };

    PlaceOnGrid(cellX, cellY);
    RecalculateCollisionBox();
}

// Snapshot implementation
std::unique_ptr<Entity> PlayerEntity::CreateSnapshot() const {
    // Create a new player with the same properties
    return std::make_unique<PlayerEntity>(*this);
}

void PlayerEntity::RestoreFromSnapshot(const Entity* snapshot) {
    Entity::RestoreFromSnapshot(snapshot);

    if (const auto* playerSnapshot = dynamic_cast<const PlayerEntity*>(snapshot)) {
        cellX = playerSnapshot->cellX;
        cellY = playerSnapshot->cellY;
        layer = playerSnapshot->layer;
        asset = playerSnapshot->asset;
        speed = playerSnapshot->speed;
        scale = playerSnapshot->scale;
        baseSize = playerSnapshot->baseSize;
    }
}

void PlayerEntity::Update(float deltaTime) {
    previousPosition = position;
    previousGridX = cellX;
    previousGridY = cellY;

    Vector2 move = {0, 0};

    if (IsKeyDown(KEY_A)) move.x -= 1.0f;
    if (IsKeyDown(KEY_D)) move.x += 1.0f;
    if (IsKeyDown(KEY_W)) move.y -= 1.0f;
    if (IsKeyDown(KEY_S)) move.y += 1.0f;

    if (Vector2Length(move) > 0.0f) {
        move = Vector2Normalize(move);
        velocity = Vector2Scale(move, speed);
    } else {
        velocity = {0, 0};
    }

    Entity::Update(deltaTime);
}

void PlayerEntity::Draw() {
    if (!active) return;

    if (asset && asset->loaded) {
        Rectangle sourceRect;
        if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
            sourceRect = asset->SpriteSourceRect;
        } else {
            sourceRect = {0, 0, (float)asset->texture.width, (float)asset->texture.height};
        }

        Rectangle destRect = {
            position.x,
            position.y,
            size.x,
            size.y
        };

        DrawTexturePro(asset->texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleV(position, size, BLUE);
    }
#ifdef _DEBUG
    Rectangle hitbox = GetBounds();
    DrawRectangleLinesEx(hitbox, 1.0f, GREEN);
#endif
}

void PlayerEntity::RestorePreviousTransform() {
    position = previousPosition;
    cellX = previousGridX;
    cellY = previousGridY;
}

void PlayerEntity::SetWorldPositionAndUpdateGrid(Vector2 worldPosition) {
    position = worldPosition;
    int tileSize = grid.GetTileSize();
    cellX = static_cast<int>(position.x / static_cast<float>(tileSize));
    cellY = static_cast<int>(position.y / static_cast<float>(tileSize));
}

void PlayerEntity::PlaceOnGrid(int gridX, int gridY) {
    cellX = gridX;
    cellY = gridY;

    const int tileSize = grid.GetTileSize();
    position.x = static_cast<float>(cellX * tileSize) + (static_cast<float>(tileSize) - size.x) / 2.0f;
    position.y = static_cast<float>(cellY * tileSize) + (static_cast<float>(tileSize) - size.y) / 2.0f;
}

void PlayerEntity::OnCollision(Entity* other) {
    velocity = {0, 0};
}