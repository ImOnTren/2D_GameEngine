#include "StaticEntity.h"
#include <cmath>

namespace {
Vector2 ResolveStaticEntityBaseSize(const Grid& grid, const Asset* asset) {
    if (!asset || !asset->loaded) {
        const float tile = static_cast<float>(grid.GetTileSize());
        return {tile, tile};
    }

    if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
        return {asset->SpriteSourceRect.width, asset->SpriteSourceRect.height};
    }

    if (asset->hasAnimations && asset->animationSet &&
        asset->animationSet->frameWidth > 0 && asset->animationSet->frameHeight > 0) {
        return {
            static_cast<float>(asset->animationSet->frameWidth),
            static_cast<float>(asset->animationSet->frameHeight)
        };
    }

    return {
        static_cast<float>(asset->texture.width),
        static_cast<float>(asset->texture.height)
    };
}

void TryPlayDefaultAnimation(Animator& animator, const AnimationSet* animationSet) {
    if (!animationSet) return;

    const Animation* idleAnim = animationSet->GetAnimation("idle");
    if (idleAnim && !idleAnim->frames.empty()) {
        animator.Play("idle");
        return;
    }

    for (const auto& [name, anim] : animationSet->animations) {
        if (anim.loop && !anim.frames.empty()) {
            animator.Play(name);
            return;
        }
    }

    for (const auto& [name, anim] : animationSet->animations) {
        if (!anim.frames.empty()) {
            animator.Play(name);
            return;
        }
    }
}
}

StaticEntity::StaticEntity(Grid& grid, Asset* asset, const int gridX, const int gridY, const int layer)
    : Entity(
          {
              static_cast<float>(gridX) * static_cast<float>(grid.GetTileSize()) +
              (static_cast<float>(grid.GetTileSize()) -
               (asset && asset->loaded ? static_cast<float>(asset->texture.width) :
                    static_cast<float>(grid.GetTileSize()))) / 2.0f,

              static_cast<float>(gridY) * static_cast<float>(grid.GetTileSize()) +
              (static_cast<float>(grid.GetTileSize()) -
               (asset && asset->loaded ? static_cast<float>(asset->texture.height) :
                    static_cast<float>(grid.GetTileSize()))) / 2.0f
          },
          {
              asset && asset->loaded ? static_cast<float>(asset->texture.width) :
                  static_cast<float>(grid.GetTileSize()),
              asset && asset->loaded ? static_cast<float>(asset->texture.height) :
                  static_cast<float>(grid.GetTileSize())
          }
      ),
      grid(grid),
      cellX(gridX),
      cellY(gridY),
      asset(asset),
      layer(layer)
{
    active = true;
    SetCollisionEnabled(false);

    const Vector2 resolvedBaseSize = ResolveStaticEntityBaseSize(grid, asset);
    const float width = resolvedBaseSize.x;
    const float height = resolvedBaseSize.y;

    size = {width, height};
    baseSize = {width, height};
    SetCollisionBox({0.0f, 0.0f}, size);

    const int tileSize = grid.GetTileSize();
    position.x = static_cast<float>(cellX * tileSize) + (static_cast<float>(tileSize) - width) / 2.0f;
    position.y = static_cast<float>(cellY * tileSize) + (static_cast<float>(tileSize) - height) / 2.0f;
}

void StaticEntity::SetScale(float newScale) {
    if (newScale <= 0.0f) {
        newScale = 0.1f;
    }
    if (newScale > 10.0f) {
        newScale = 10.0f;
    }

    scale = newScale;

    size.x = baseSize.x * scale;
    size.y = baseSize.y * scale;
    SetCollisionBox({0.0f, 0.0f}, size);

    const int tileSize = grid.GetTileSize();
    position.x = static_cast<float>(cellX) * static_cast<float>(tileSize) +
                 (static_cast<float>(tileSize) - size.x) / 2.0f;
    position.y = static_cast<float>(cellY) * static_cast<float>(tileSize) +
                 (static_cast<float>(tileSize) - size.y) / 2.0f;
}

void StaticEntity::Update(float deltaTime) {
    if (asset && asset->hasAnimations) {
        if (animator.GetAnimationSet() == nullptr && asset->animationSet) {
            animator.SetAnimationSet(asset->animationSet.get());
            TryPlayDefaultAnimation(animator, asset->animationSet.get());
        }
        animator.Update(deltaTime);
    }
}

void StaticEntity::Draw() {
    if (!active || !asset || !asset->loaded) return;

    Rectangle sourceRect;
    Texture2D drawTexture = asset->texture;

    if (asset->hasAnimations && animator.IsPlaying()) {
        sourceRect = animator.GetCurrentFrameRect();
        Texture2D animTexture = animator.GetTexture();
        if (animTexture.id != 0) {
            drawTexture = animTexture;
        }
    }
    else if (asset->hasSelectedFrame) {
        sourceRect = asset->SpriteSourceRect;
    }
    else {
        sourceRect = {0, 0, (float)drawTexture.width, (float)drawTexture.height};
    }

    const Rectangle destRect = {
        position.x,
        position.y,
        size.x,
        size.y
    };

    Rectangle drawDest = destRect;
    if (asset->hasAnimations && animator.IsPlaying() && animator.IsFlippedHorizontal()) {
        drawDest.width = -drawDest.width;
    }

    DrawTexturePro(drawTexture, sourceRect, drawDest, {0, 0}, 0.0f, WHITE);
}

std::unique_ptr<Entity> StaticEntity::CreateSnapshot() const {
    auto newSnapshot = std::make_unique<StaticEntity>(grid, asset, cellX, cellY, layer);
    newSnapshot->SetPosition(position);
    newSnapshot->SetVelocity(velocity);
    newSnapshot->SetSize(size);
    newSnapshot->SetActive(active);
    newSnapshot->SetScale(scale);
    return newSnapshot;
}

void StaticEntity::RestoreFromSnapshot(const Entity *snapshot) {
    if (const auto* staticSnapshot = dynamic_cast<const StaticEntity*>(snapshot)) {
        Entity::RestoreFromSnapshot(snapshot);
        cellX = staticSnapshot->cellX;
        cellY = staticSnapshot->cellY;
        layer = staticSnapshot->layer;
        scale = staticSnapshot->scale;
        baseSize = staticSnapshot->baseSize;
    }
}
