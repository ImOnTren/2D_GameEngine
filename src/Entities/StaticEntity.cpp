#include "StaticEntity.h"
#include <cmath>

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

    float width, height;

    if (asset && asset->loaded) {
        // Check if asset has a selected frame
        if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
            // Use selected frame dimensions
            width = asset->SpriteSourceRect.width;
            height = asset->SpriteSourceRect.height;
        } else {
            // Use full texture dimensions
            width = static_cast<float>(asset->texture.width);
            height = static_cast<float>(asset->texture.height);
        }
    } else {
        // Fallback to grid tile size
        width = static_cast<float>(grid.GetTileSize());
        height = static_cast<float>(grid.GetTileSize());
    }

    // Set size
    size = {width, height};
    baseSize = {width, height};

    // Center in grid cell
    const int tileSize = grid.GetTileSize();
    position.x = static_cast<float>(cellX * tileSize) + (static_cast<float>(tileSize) - width) / 2.0f;
    position.y = static_cast<float>(cellY * tileSize) + (static_cast<float>(tileSize) - height) / 2.0f;
}

void StaticEntity::SetScale(float newScale) {
    if (newScale <= 0.0f) {
        newScale = 0.1f;  // Minimum scale
    }
    if (newScale > 10.0f) {
        newScale = 10.0f;  // Maximum scale
    }

    scale = newScale;

    // Update size based on scale while maintaining aspect ratio
    size.x = baseSize.x * scale;
    size.y = baseSize.y * scale;

    // Recenter the entity in its grid cell
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
            animator.Play("idle");
        }
        animator.Update(deltaTime);
    }
}

void StaticEntity::Draw() {
    if (!active || !asset || !asset->loaded) return;

    Rectangle sourceRect;

    // ROZHODNUTIE: Čo ideme kresliť?
    if (asset->hasAnimations && animator.IsPlaying()) {
        // Ak hrá animácia (Play Mode), zoberieme aktuálny snímok
        sourceRect = animator.GetCurrentFrameRect();
    }
    else if (asset->hasSelectedFrame) {
        // Ak sme v Editore, kreslíme len vybraný statický obrázok
        sourceRect = asset->SpriteSourceRect;
    }
    else {
        sourceRect = { 0, 0, (float)asset->texture.width, (float)asset->texture.height };
    }

    // CIEĽ: Tu sa zachová tvoje scaleovanie
    // size.x a size.y už v sebe majú započítaný scale z funkcie SetScale()
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

    DrawTexturePro(asset->texture, sourceRect, drawDest, {0, 0}, 0.0f, WHITE);
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
        position = staticSnapshot->position;
        velocity = staticSnapshot->velocity;
        size = staticSnapshot->size;
        rotation = staticSnapshot->rotation;
        active = staticSnapshot->active;
        cellX = staticSnapshot->cellX;
        cellY = staticSnapshot->cellY;
        layer = staticSnapshot->layer;
        scale = staticSnapshot->scale;
        baseSize = staticSnapshot->baseSize;
    }
}
