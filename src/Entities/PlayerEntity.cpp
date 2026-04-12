#include "PlayerEntity.h"
#include <cmath>

AnimationDirection PlayerEntity::ResolveDirectionFromVelocity(const Vector2& vel, const AnimationDirection fallback) {
    if (std::abs(vel.x) < 0.0001f && std::abs(vel.y) < 0.0001f) {
        return fallback;
    }

    if (std::abs(vel.x) > std::abs(vel.y)) {
        return vel.x >= 0.0f ? AnimationDirection::RIGHT : AnimationDirection::LEFT;
    }

    return vel.y >= 0.0f ? AnimationDirection::DOWN : AnimationDirection::UP;
}

std::string PlayerEntity::BuildDirectionalName(const std::string& baseName, const AnimationDirection direction) {
    switch (direction) {
        case AnimationDirection::DOWN:  return baseName + "_down";
        case AnimationDirection::UP:    return baseName + "_up";
        case AnimationDirection::RIGHT: return baseName + "_right";
        case AnimationDirection::LEFT:  return baseName + "_left";
        case AnimationDirection::NONE:
        default: return baseName;
    }
}

const Animation* PlayerEntity::FindDirectionalAnimation(const std::string& baseName, const AnimationDirection direction) const {
    if (!asset || !asset->animationSet) return nullptr;

    auto getAnim = [&](const std::string& name) -> const Animation* {
        return asset->animationSet->GetAnimation(name);
    };

    const std::string primaryName = BuildDirectionalName(baseName, direction);
    if (const Animation* anim = getAnim(primaryName)) return anim;

    // Backward compatibility for previously misnamed entries like "idle_down_down".
    if (const Animation* anim = getAnim(BuildDirectionalName(primaryName, direction))) return anim;

    if (direction == AnimationDirection::LEFT) {
        // Fallback: if there is no explicit *_left animation, use *_right and flip at render time.
        if (const Animation* anim = getAnim(BuildDirectionalName(baseName, AnimationDirection::RIGHT))) return anim;
    }

    return nullptr;
}

bool PlayerEntity::PlayDirectionalTriggered(const std::string& baseName, const AnimationTrigger trigger,
                                            const AnimationDirection direction, const bool forceRestart) {
    const Animation* anim = FindDirectionalAnimation(baseName, direction);
    if (!anim) return false;
    if (anim->trigger != trigger) return false;
    animator.PlayDirectional(baseName, direction, forceRestart);
    return true;
}

void PlayerEntity::EnsureAnimatorInitialized() {
    if (!asset || !asset->hasAnimations || !asset->animationSet) return;
    if (animator.GetAnimationSet() != asset->animationSet.get()) {
        animator.SetAnimationSet(asset->animationSet.get());
    }
}

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
    EnsureAnimatorInitialized();
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
      animator(other.animator),
      lastFacingDirection(other.lastFacingDirection),
      scale(other.scale),
      baseSize(other.baseSize) {
    EnsureAnimatorInitialized();
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
        lastFacingDirection = playerSnapshot->lastFacingDirection;
        scale = playerSnapshot->scale;
        baseSize = playerSnapshot->baseSize;
        EnsureAnimatorInitialized();
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

    if (asset && asset->hasAnimations && asset->animationSet) {
        EnsureAnimatorInitialized();

        lastFacingDirection = ResolveDirectionFromVelocity(velocity, lastFacingDirection);
        const bool moving = Vector2Length(velocity) > 0.001f;
        const bool sprinting = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        const bool attackPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE);

        const Animation* currentAnim = asset->animationSet->GetAnimation(animator.GetCurrentAnimationName());
        const bool currentIsEvent = currentAnim && currentAnim->trigger == AnimationTrigger::EVENT &&
                                    animator.IsPlaying() && !animator.IsFinished();

        if (attackPressed) {
            PlayDirectionalTriggered("attack", AnimationTrigger::EVENT, lastFacingDirection, true);
        } else if (!currentIsEvent) {
            bool played = false;
            if (moving) {
                if (sprinting) {
                    played = PlayDirectionalTriggered("run", AnimationTrigger::INPUT, lastFacingDirection);
                }
                if (!played) {
                    played = PlayDirectionalTriggered("walk", AnimationTrigger::INPUT, lastFacingDirection);
                }
                if (!played && FindDirectionalAnimation("walk", lastFacingDirection)) {
                    animator.PlayDirectional("walk", lastFacingDirection);
                    played = true;
                }
            }

            if (!played) {
                played = PlayDirectionalTriggered("idle", AnimationTrigger::IDLE, lastFacingDirection);
            }
            if (!played && FindDirectionalAnimation("idle", lastFacingDirection)) {
                animator.PlayDirectional("idle", lastFacingDirection);
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
            if (!played && asset->animationSet->GetAnimation("run")) {
                animator.Play("run");
                played = true;
            }
            if (!played && asset->animationSet->GetAnimation("attack")) {
                animator.Play("attack");
                played = true;
            }
        }

        animator.Update(deltaTime);
    }

    Entity::Update(deltaTime);
}

void PlayerEntity::Draw() {
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
            destRect.width = -destRect.width;
        }

        DrawTexturePro(drawTexture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
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
