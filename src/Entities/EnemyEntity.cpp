#include "EnemyEntity.h"
#include "PlayerEntity.h"
#include "raylib.h"
#include <cmath>

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
    EnsureAnimatorInitialized();
}

EnemyEntity::EnemyEntity(const EnemyEntity& other)
    : Entity(other),
      grid(other.grid),
      cellX(other.cellX),
      cellY(other.cellY),
      layer(other.layer),
      asset(other.asset),
      speed(other.speed),
      animator(other.animator),
      lastFacingDirection(other.lastFacingDirection),
      scale(other.scale),
      baseSize(other.baseSize) {
    EnsureAnimatorInitialized();
}

void EnemyEntity::EnsureAnimatorInitialized()
{
    if (!asset || !asset->hasAnimations || !asset->animationSet) return;
    if (animator.GetAnimationSet() != asset->animationSet.get()) {
        animator.SetAnimationSet(asset->animationSet.get());
    }
}

AnimationDirection EnemyEntity::ResolveDirectionFromVelocity(const Vector2& vel, AnimationDirection fallback)
{
    if (Vector2Length(vel) <= 0.0001f) {
        return fallback;
    }

    if (std::abs(vel.x) > std::abs(vel.y))
    {
        return vel.x >= 0.0f ? AnimationDirection::RIGHT : AnimationDirection::LEFT;
    }
    return vel.y >= 0.0f ? AnimationDirection::DOWN : AnimationDirection::UP;
}

std::string EnemyEntity::BuildDirectionalName(const std::string& baseName, AnimationDirection direction)
{
    switch (direction) {
        case AnimationDirection::UP: return baseName + "_up";
        case AnimationDirection::DOWN: return baseName + "_down";
        case AnimationDirection::LEFT: return baseName + "_left";
        case AnimationDirection::RIGHT: return baseName + "_right";
        default: return baseName;
    }
}

const Animation* EnemyEntity::FindDirectionalAnimation(const std::string& baseName, AnimationDirection direction) const {
    if (!asset || !asset->hasAnimations || !asset->animationSet) return nullptr;

    std::string directionalName = BuildDirectionalName(baseName, direction);
    if (const Animation* anim = asset->animationSet->GetAnimation(directionalName)) return anim;

    // Backward compatibility for previously misnamed entries like "idle_down_down".
    if (const Animation* anim = asset->animationSet->GetAnimation(BuildDirectionalName(directionalName, direction))) return anim;

    if (direction == AnimationDirection::LEFT || direction == AnimationDirection::RIGHT) {
        // Fallback: if one horizontal direction is missing, use the opposite one.
        const AnimationDirection opposite =
            (direction == AnimationDirection::LEFT) ? AnimationDirection::RIGHT : AnimationDirection::LEFT;
        if (const Animation* anim = asset->animationSet->GetAnimation(BuildDirectionalName(baseName, opposite))) return anim;
    }
    return nullptr;
}

bool EnemyEntity::PlayDirectionalTriggered(const std::string& baseName, AnimationTrigger trigger, AnimationDirection direction, bool forceRestart)
{
    const Animation* anim = FindDirectionalAnimation(baseName, direction);
    if (!anim) return false;
    if (anim->trigger != trigger) return false;
    animator.PlayDirectional(baseName, direction, forceRestart);
    return true;
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

    Vector2 previousPosition = position;

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
    bool shouldStopBecauseOfPlayerContact = false;
    if (CheckCollisionRecs(playerBounds, myBounds))
    {
        direction = {0.0f, 0.0f};
        shouldStopBecauseOfPlayerContact = true;
    }

    if (Vector2Length(direction) > 0.0001f)
    {
        direction = Vector2Normalize(direction);
    }
    Vector2 step = Vector2Scale(direction, speed * deltaTime);

    if (shouldStopBecauseOfPlayerContact) {
        step = {0, 0};
    }

    Vector2 candidatePos = position;
    bool xBlockedByPlayer = false;
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
        else {
            xBlockedByPlayer = true;
        }
    }

    bool yBlockedByPlayer = false;
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
        else {
            yBlockedByPlayer = true;
        }
    }

    if (asset && asset->hasAnimations && asset->animationSet) {
        Vector2 actualDelta = { position.x - previousPosition.x, position.y - previousPosition.y };
        Vector2 actualVelocity = (deltaTime > 0.0001f) ? Vector2Scale(actualDelta, 1.0f / deltaTime) : Vector2{0,0};
        bool blockedByPlayer = xBlockedByPlayer || yBlockedByPlayer;
        bool moving = shouldStopBecauseOfPlayerContact || blockedByPlayer ? false : Vector2Length(actualVelocity) > 0.001f;
        bool played = false;
        lastFacingDirection = ResolveDirectionFromVelocity(actualVelocity, lastFacingDirection);
        if (moving)
        {
            played = PlayDirectionalTriggered("walk", AnimationTrigger::INPUT, lastFacingDirection);

            if (!played && FindDirectionalAnimation("walk", lastFacingDirection)) {
                animator.PlayDirectional("walk", lastFacingDirection);
                played = true;
            }
            if (!played && FindDirectionalAnimation("idle", lastFacingDirection)) {
                animator.PlayDirectional("idle", lastFacingDirection);
                played = true;
            }
            if (!played && asset->animationSet->GetAnimation("walk")) {
                animator.Play("walk");
                played = true;
            }
            if (!played && asset->animationSet->GetAnimation("idle")) {
                animator.Play("idle");
                played = true;
            }
        }
        else
        {
            played = PlayDirectionalTriggered("idle", AnimationTrigger::IDLE, lastFacingDirection);
            if (!played && FindDirectionalAnimation("idle", lastFacingDirection)) {
                animator.PlayDirectional("idle", lastFacingDirection);
                played = true;
            }
            if (!played && FindDirectionalAnimation("walk", lastFacingDirection)) {
                animator.PlayDirectional("walk", lastFacingDirection);
                played = true;
            }
            if (!played && asset->animationSet->GetAnimation("idle")) {
                animator.Play("idle");
                played = true;
            }
            if (!played && asset->animationSet->GetAnimation("walk")) {
                animator.Play("walk");
                played = true;
            }
        }
        animator.Update(deltaTime);
    }

    Entity::Update(deltaTime);

    int tileSize = grid.GetTileSize();
    cellX = static_cast<int>(position.x / tileSize);
    cellY = static_cast<int>(position.y / tileSize);
}

void EnemyEntity::Draw() {
    if (!active) return;

    if (asset && asset->loaded) {
        Rectangle sourceRect;
        Texture2D drawTexture = asset->texture;
        if (asset->hasAnimations && animator.IsPlaying()) {
            sourceRect = animator.GetCurrentFrameRect();
            Texture2D animTex = animator.GetTexture();
            if (animTex.id != 0) {
                drawTexture = animTex;
            }
        } else if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
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

        if (asset->hasAnimations && animator.IsPlaying() && animator.IsFlippedHorizontal()) {
            sourceRect.width = -sourceRect.width;
        }

        DrawTexturePro(drawTexture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
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
        lastFacingDirection = enemySnapshot->lastFacingDirection;
        baseSize = enemySnapshot->baseSize;
        EnsureAnimatorInitialized();
    }
}
