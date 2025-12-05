#include "Scene.h"
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

std::vector<std::unique_ptr<Entity>>& Scene::GetEditModeEntities() {
    return editModeEntities;
}

std::vector<std::unique_ptr<Entity>>& Scene::GetPlayModeSnapshots(){
    return playModeSnapshots;
}

const Rectangle& Scene::GetEditModeCameraArea() const{
    return editModeCameraArea;
}

void Scene::SetEditModeCameraArea(const Rectangle& newArea)
{
    editModeCameraArea = newArea;
}

const Rectangle& Scene::GetPlayModeCameraArea() const{
    return playModeCameraArea;
}

void Scene::SetPlayModeCameraArea(const Rectangle& newArea)
{
    playModeCameraArea = newArea;
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