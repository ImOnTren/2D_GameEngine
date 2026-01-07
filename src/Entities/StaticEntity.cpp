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
}


void StaticEntity::Update(float deltaTime) {

}

void StaticEntity::Draw() {
    if (!active || !asset->loaded) return;

    std::floor(position.x);
    std::floor(position.y);

    const Rectangle sourceRect = { 0, 0,
        static_cast<float>(asset->texture.width),
        static_cast<float>(asset->texture.height) };
    const Rectangle destRect = { position.x, position.y,
        static_cast<float>(asset->texture.width),
        static_cast<float>(asset->texture.height) };

    DrawTexturePro(asset->texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
}

std::unique_ptr<Entity> StaticEntity::CreateSnapshot() const {
    auto newSnapshot = std::make_unique<StaticEntity>(grid, asset, cellX, cellY, layer);
    newSnapshot->SetPosition(position);
    newSnapshot->SetVelocity(velocity);
    newSnapshot->SetSize(size);
    newSnapshot->SetActive(active);
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
    }
}
