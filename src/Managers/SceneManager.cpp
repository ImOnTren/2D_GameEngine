#include "SceneManager.h"
#include "Entities/TileMap.h"
#include "Entities/PlayerEntity.h"
#include "Entities/Entity.h"

Scene::Scene(const std::string& id, const std::string& displayName)
    : id(id), name(displayName), tileMap(std::make_unique<TileMap>()) {

}

const std::string& Scene::GetId() const {
    return id;
}

const std::string& Scene::GetName() const {
    return name;
}

void Scene::SetName(const std::string& newName) {
    name = newName;
}

TileMap& Scene::GetTileMap() {
    return *tileMap;
}

const TileMap& Scene::GetTileMap() const {
    return *tileMap;
}

std::vector<std::unique_ptr<Entity>>& Scene::GetEntities() {
    return editModeEntities;
}

const std::vector<std::unique_ptr<Entity>>& Scene::GetEntities() const {
    return editModeEntities;
}

void Scene::Clear() {
    tileMap->Clear();
    editModeEntities.clear();
}

PlayerEntity* Scene::FindPlayer() {
    for (auto& entity : editModeEntities) {
        if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
            return player;
        }
    }
    return nullptr;
}

const PlayerEntity* Scene::FindPlayer() const {
    for (const auto& entity : editModeEntities) {
        if (auto player = dynamic_cast<const PlayerEntity*>(entity.get())) {
            return player;
        }
    }
    return nullptr;
}