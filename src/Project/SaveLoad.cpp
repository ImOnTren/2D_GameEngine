#include <fstream>
#include "Project/SaveLoad.h"
#include "Scene/Scene.h"
#include "Entities/TileMap.h"
#include "Entities/PlayerEntity.h"
#include "Entities/EnemyEntity.h"
#include "Engine.h"
#include "UI/UI.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

static json RectToJson(const Rectangle& rect) {
    json j;
    j["x"] = rect.x;
    j["y"] = rect.y;
    j["width"] = rect.width;
    j["height"] = rect.height;
    return j;
}

static Rectangle JsonToRect(const json& j) {
    Rectangle rect{0,0,0,0};
    rect.x = j.value("x", 0.0f);
    rect.y = j.value("y", 0.0f);
    rect.width = j.value("width", 0.0f);
    rect.height = j.value("height", 0.0f);
    return rect;
}

static json SerializeTileMap(const TileMap& tileMap) {
    json j;
    j = json::array();
    for (const auto& [key, tileData] : tileMap.GetAllTiles()) {
        int x = static_cast<int>(key >> 32);
        int y = static_cast<int>(key & 0xFFFFFFFF);
        std::vector<TileData> tilesAtPos = tileMap.GetTilesAtPosition(x, y);
        for (const auto& tile : tilesAtPos) {
            json tileJson;
            tileJson["x"] = x;
            tileJson["y"] = y;
            tileJson["tileID"] = tile.tileID;
            tileJson["tileIndex"] = tile.tileIndex;
            tileJson["isSolid"] = tile.isSolid;
            tileJson["isSceneSwitcher"] = tile.isSceneSwitcher;
            tileJson["targetSceneID"] = tile.targetSceneID;
            tileJson["triggerKey"] = tile.triggerKey;
            tileJson["layer"] = tile.layer;
            j.push_back(tileJson);
        }
    }
    return j;
}

static json SerializeEntities(const std::vector<std::unique_ptr<Entity>>& entities) {
    json j;
    j = json::array();
    for (const auto& entity : entities) {
        json entityJson;
        if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
            entityJson["type"] = "Player";
            entityJson["gridX"] = player->GetGridX();
            entityJson["gridY"] = player->GetGridY();
        } else if (auto enemy = dynamic_cast<EnemyEntity*>(entity.get())) {
            entityJson["type"] = "Enemy";
            entityJson["gridX"] = enemy->GetGridX();
            entityJson["gridY"] = enemy->GetGridY();
        }
        j.push_back(entityJson);
    }
    return j;
}

bool SaveLoad::SaveProject(Engine& engine, const std::string& filename) {
    try {
        json projectRoot;
        projectRoot["version"] = "1.1";

        json engineJson;
        engineJson["selectedResolutionIndex"] = engine.GetSelectedResolutionIndex();
        engineJson["currentSceneIndex"] = engine.GetCurrentSceneIndex();
        engineJson["nextSceneId"] = engine.GetNextSceneId();
        engineJson["startingSceneID"] = engine.GetStartingSceneID();
        engineJson["totalLayers"] = engine.GetTotalLayers();
        engineJson["activeLayer"] = engine.GetCurrentTileLayer();

        // Save layer visibility
        json layerVisibilityJson = json::array();
        for (int i = 0; i < engine.GetTotalLayers(); i++) {
            layerVisibilityJson.push_back(engine.IsLayerVisible(i));
        }
        engineJson["layerVisibility"] = layerVisibilityJson;

        projectRoot["engine"] = engineJson;

        json scenesJson = json::array();
        const auto& scenes = engine.GetAllScenes();
        for (const auto& [sceneID, scenePtr] : scenes) {
            json sceneJson;
            sceneJson["id"] = scenePtr->GetId();
            sceneJson["name"] = scenePtr->GetName();
            sceneJson["editModeCameraArea"] = RectToJson(scenePtr->GetEditModeCameraArea());
            sceneJson["playModeCameraArea"] = RectToJson(scenePtr->GetPlayModeCameraArea());
            sceneJson["tiles"] = SerializeTileMap(scenePtr->GetTileMap());
            sceneJson["entities"] = SerializeEntities(scenePtr->GetEditModeEntities());
            scenesJson.push_back(sceneJson);
        }
        projectRoot["scenes"] = scenesJson;
        std::ofstream fout(filename);
        if (fout) {
            fout << projectRoot.dump(4);
            UI::SetDebugMessage("Successfully created project.json");
            return true;
        }
        UI::SetDebugMessage("[ERROR] Could not open file for writing: " + filename);
        return false;
    } catch (const std::exception& e) {
        UI::SetDebugMessage("[ERROR] Failed to save project: " + std::string(e.what()));
        return false;
    }
}

bool SaveLoad::LoadProject(Engine& engine, const std::string& filepath) {
    try {
        std::ifstream fin(filepath);
        if (!fin) {
            UI::SetDebugMessage("[ERROR] Could not open project file: " + filepath);
            return false;
        }

        json projectRoot;
        fin >> projectRoot;

        if (!projectRoot.contains("engine") && !projectRoot.contains("scenes")) {
            UI::SetDebugMessage("[ERROR] Specified file does not contain 'engine' and 'scenes' objects: " + filepath);
            return false;
        }

        json scenesJson = projectRoot["scenes"];
        json engineJson = projectRoot["engine"];

        auto& scenes = engine.GetAllScenes();
        scenes.clear();

        for (const auto& scene : scenesJson) {
            std::string sceneId = scene.value("id", "");
            std::string sceneName = scene.value("name", "Unnamed Scene");

            auto newScene = std::make_unique<Scene>(sceneId, sceneName);

            if (scene.contains("tiles") && scene["tiles"].is_array()) {
                TileMap& newTileMap = newScene->GetTileMap();
                for (const auto& jsonTileMap : scene["tiles"]) {
                    int x = jsonTileMap.value("x", 0);
                    int y = jsonTileMap.value("y", 0);
                    TileData tileData;
                    tileData.tileID = jsonTileMap.value("tileID", "");
                    tileData.tileIndex = jsonTileMap.value("tileIndex", 0);
                    tileData.isSolid = jsonTileMap.value("isSolid", false);
                    tileData.isSceneSwitcher = jsonTileMap.value("isSceneSwitcher", false);
                    tileData.targetSceneID = jsonTileMap.value("targetSceneID", "");
                    tileData.triggerKey = jsonTileMap.value("triggerKey", 0);
                    tileData.layer = jsonTileMap.value("layer", 0);
                    newTileMap.SetTile(x, y, tileData, tileData.layer);
                }
            }
            if (scene.contains("entities") && scene["entities"].is_array()) {
                auto& editModeEntities = newScene->GetEditModeEntities();
                for (const auto& jsonEditModeEntity : scene["entities"]) {
                    if (jsonEditModeEntity.contains("type")) {
                        if (jsonEditModeEntity["type"] == "Player") {
                            int gridX = jsonEditModeEntity.value("gridX", 0);
                            int gridY = jsonEditModeEntity.value("gridY", 0);
                            editModeEntities.push_back(
                                std::make_unique<PlayerEntity>(engine.GetGrid(), gridX, gridY)
                            );
                        }
                        else if (jsonEditModeEntity["type"] == "Enemy") {
                            int gridX = jsonEditModeEntity.value("gridX", 0);
                            int gridY = jsonEditModeEntity.value("gridY", 0);
                            editModeEntities.push_back(
                                std::make_unique<EnemyEntity>(engine.GetGrid(), gridX, gridY)
                            );
                        }
                    }
                }
            }
            if (engineJson.contains("totalLayers")) {
                int totalLayers = engineJson.value("totalLayers", 5);
                engine.SetTotalLayers(totalLayers);
            }

            if (engineJson.contains("activeLayer")) {
                engine.SetCurrentTileLayer(engineJson.value("activeLayer", 0));
            }

            if (engineJson.contains("layerVisibility")) {
                const auto& visibilityArray = engineJson["layerVisibility"];
                if (visibilityArray.is_array()) {
                    for (size_t i = 0; i < visibilityArray.size(); i++) {
                        if (i < static_cast<size_t>(engine.GetTotalLayers())) {
                            engine.SetLayerVisible(static_cast<int>(i), visibilityArray[i]);
                        }
                    }
                }
            }
            if (scene.contains("editModeCameraArea")) {
                Rectangle editModeCameraArea = JsonToRect(scene["editModeCameraArea"]);
                newScene->SetEditModeCameraArea(editModeCameraArea);
            } else if (scene.contains("editModeCamera")) {
                Rectangle editModeCameraArea = JsonToRect(scene["editCamera"]);
                newScene->SetEditModeCameraArea(editModeCameraArea);
            }
            if (scene.contains("playModeCameraArea")) {
                Rectangle playModeCameraArea = JsonToRect(scene["playModeCameraArea"]);
                newScene->SetPlayModeCameraArea(playModeCameraArea);
            } else if (scene.contains("playCamera")) {
                Rectangle playModeCameraArea = JsonToRect(scene["playCamera"]);
                newScene->SetPlayModeCameraArea(playModeCameraArea);
            }
            scenes[sceneId] = std::move(newScene);
        }
        if (engineJson.contains("selectedResolutionIndex")) {
            engine.SetSelectedResolutionIndex(engineJson.value("selectedResolutionIndex", 1));
        }
        else {
            UI::SetDebugMessage("[WARNING] 'selectedResolutionIndex' not found in project file. Using default.");
        }
        if (engineJson.contains("currentSceneIndex")) {
            engine.SetCurrentSceneByIndex(engineJson.value("currentSceneIndex", 1));
        }
        else {
            UI::SetDebugMessage("[WARNING] 'currentSceneIndex' not found in project file. Using default.");
        }
        if (engineJson.contains("nextSceneId")) {
            engine.SetNextSceneId(engineJson.value("nextSceneId", 1));
        }
        else {
            UI::SetDebugMessage("[WARNING] 'nextSceneId' not found in project file. Using default.");
        }
        if (engineJson.contains("startingSceneID")) {
            engine.SetStartingSceneID(engineJson.value("startingSceneID", std::string("")));
        }
        else if (engineJson.contains("startingSceneIndex")) {
            engine.SetStartingSceneID(engineJson.value("startingSceneIndex", std::string("")));
        }
        else {
            UI::SetDebugMessage("[WARNING] 'startingSceneID' not found in project file. Using default.");
        }
        engine.currentMode = Engine::Mode::EDIT;
        engine.StopPlayMode();
        engine.currentTool = Engine::ToolState::NONE;
        UI::SetDebugMessage("Successfully loaded project from " + filepath);

        return true;
    } catch (const std::exception& e) {
        UI::SetDebugMessage("[ERROR] Failed to load project: " + std::string(e.what()));
        return false;
    }
}
