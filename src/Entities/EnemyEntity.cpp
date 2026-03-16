#include "EnemyEntity.h"
#include "PlayerEntity.h"
#include "raylib.h"

EnemyEntity::EnemyEntity(Grid& grid, Asset* asset, const int gridX, const int gridY, const int layer)
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

    baseSize = { w, h };
    size = baseSize;
    scale = 1.0f;

    PlaceOnGrid(gridX, gridY);
    RecalculateCollisionBox();
}

EnemyEntity::EnemyEntity(const EnemyEntity& other)
    : Entity(other),
      grid(other.grid),
      cellX(other.cellX),
      cellY(other.cellY),
      layer(other.layer),
      asset(other.asset),
      speed(other.speed),
      scale(other.scale),
      baseSize(other.baseSize) {
}

void EnemyEntity::RecalculateCollisionBox() {
    float hitboxWidth  = size.x * 0.45f;
    float hitboxHeight = size.y * 0.75f;
    float hitboxOffsetX = (size.x - hitboxWidth) * 0.5f;
    float hitboxOffsetY = size.y - hitboxHeight - (4.0f * scale);

    SetCollisionBox({ hitboxOffsetX, hitboxOffsetY }, { hitboxWidth, hitboxHeight });
}

void EnemyEntity::SetScale(float newScale) {
    if (newScale < 0.25f) newScale = 0.25f;
    if (newScale > 4.0f)  newScale = 4.0f;

    scale = newScale;
    size = { baseSize.x * scale, baseSize.y * scale };

    PlaceOnGrid(cellX, cellY);      // keeps it aligned in its cell
    RecalculateCollisionBox();      // keeps hitbox synced
}

void EnemyEntity::Update(float deltaTime) {
    Entity::Update(deltaTime);
}

void EnemyEntity::Update(float deltaTime, const PlayerEntity* player) {
    if (!player) return;

    Rectangle playerBounds = player->GetBounds();
    Rectangle myBounds = GetBounds();

    Vector2 myCenter = {
        myBounds.x + myBounds.width * 0.5f,
        myBounds.y + myBounds.height * 0.5f
    };

    Vector2 playerCenter = {
        playerBounds.x + playerBounds.width * 0.5f,
        playerBounds.y + playerBounds.height * 0.5f
    };

    Vector2 direction = Vector2Subtract(playerCenter, myCenter);

    if (Vector2Length(direction) <= 0.0001f) {
        return;
    }

    direction = Vector2Normalize(direction);
    Vector2 step = Vector2Scale(direction, speed * deltaTime);

    Vector2 candidatePos = position;

    if (step.x != 0.0f) {
        candidatePos.x += step.x;
        Rectangle nextXBounds = {
            candidatePos.x + collisionOffset.x,
            position.y + collisionOffset.y,
            collisionSize.x,
            collisionSize.y
        };

        if (!CheckCollisionRecs(nextXBounds, playerBounds)) {
            position.x = candidatePos.x;
        }
    }

    if (step.y != 0.0f) {
        candidatePos = position;
        candidatePos.y += step.y;
        Rectangle nextYBounds = {
            position.x + collisionOffset.x,
            candidatePos.y + collisionOffset.y,
            collisionSize.x,
            collisionSize.y
        };

        if (!CheckCollisionRecs(nextYBounds, playerBounds)) {
            position.y = candidatePos.y;
        }
    }

    int tileSize = grid.GetTileSize();
    cellX = static_cast<int>(position.x / tileSize);
    cellY = static_cast<int>(position.y / tileSize);
}

void EnemyEntity::Draw() {
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
        DrawRectangleV(position, size, RED);
    }
#ifdef _DEBUG
    Rectangle hitbox = GetBounds();
    DrawRectangleLinesEx(hitbox, 1.0f, GREEN);
#endif
}

void EnemyEntity::PlaceOnGrid(int gridX, int gridY) {
    cellX = gridX;
    cellY = gridY;

    const int tileSize = grid.GetTileSize();
    position.x = static_cast<float>(cellX * tileSize) + (static_cast<float>(tileSize) - size.x) / 2.0f;
    position.y = static_cast<float>(cellY * tileSize) + (static_cast<float>(tileSize) - size.y) / 2.0f;
}

std::unique_ptr<Entity> EnemyEntity::CreateSnapshot() const {
    // Create a new enemy with the same properties
    return std::make_unique<EnemyEntity>(*this);
}

void EnemyEntity::OnCollision(Entity* other) {
    velocity = {0, 0};
}

void EnemyEntity::RestoreFromSnapshot(const Entity* snapshot) {
    Entity::RestoreFromSnapshot(snapshot);

    if (const auto* enemySnapshot = dynamic_cast<const EnemyEntity*>(snapshot)) {
        cellX = enemySnapshot->cellX;
        cellY = enemySnapshot->cellY;
        layer = enemySnapshot->layer;
        asset = enemySnapshot->asset;
        speed = enemySnapshot->speed;
        scale = enemySnapshot->scale;
        baseSize = enemySnapshot->baseSize;
    }
}