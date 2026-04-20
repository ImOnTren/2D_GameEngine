#include <fstream>
#include "Project/SaveLoad.h"
#include "Scene/Scene.h"
#include "Entities/TileMap.h"
#include "Entities/PlayerEntity.h"
#include "Entities/EnemyEntity.h"
#include "Entities/StaticEntity.h"
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

static json SerializeAnimatedTileDefinitions(const Engine& engine) {
    json result = json::array();
    for (const auto& def : engine.GetAnimatedTileDefinitions()) {
        json defJson;
        defJson["id"] = def.id;
        defJson["loop"] = def.loop;

        json baseTilesJson = json::array();
        for (const auto& base : def.baseTiles) {
            json baseJson;
            baseJson["assetID"] = base.assetID;
            baseJson["tileIndex"] = base.tileIndex;
            baseTilesJson.push_back(baseJson);
        }
        defJson["baseTiles"] = baseTilesJson;

        json framesJson = json::array();
        for (const auto& frame : def.frames) {
            json frameJson;
            frameJson["assetID"] = frame.assetID;
            frameJson["tileIndex"] = frame.tileIndex;
            frameJson["duration"] = frame.duration;
            framesJson.push_back(frameJson);
        }
        defJson["frames"] = framesJson;
        result.push_back(defJson);
    }
    return result;
}

static std::vector<Engine::AnimatedTileDefinition> DeserializeAnimatedTileDefinitions(const json& tileAnimationsJson) {
    std::vector<Engine::AnimatedTileDefinition> definitions;
    if (!tileAnimationsJson.is_array()) {
        return definitions;
    }

    for (const auto& defJson : tileAnimationsJson) {
        if (!defJson.is_object()) continue;

        Engine::AnimatedTileDefinition def;
        def.id = defJson.value("id", "");
        def.loop = defJson.value("loop", true);

        if (defJson.contains("baseTiles") && defJson["baseTiles"].is_array()) {
            for (const auto& baseJson : defJson["baseTiles"]) {
                Engine::TileRef baseRef;
                baseRef.assetID = baseJson.value("assetID", "");
                baseRef.tileIndex = baseJson.value("tileIndex", 0);
                if (!baseRef.assetID.empty()) {
                    def.baseTiles.push_back(baseRef);
                }
            }
        }

        if (defJson.contains("frames") && defJson["frames"].is_array()) {
            for (const auto& frameJson : defJson["frames"]) {
                Engine::AnimatedTileFrame frame;
                frame.assetID = frameJson.value("assetID", "");
                frame.tileIndex = frameJson.value("tileIndex", 0);
                frame.duration = frameJson.value("duration", 0.2);
                if (!frame.assetID.empty()) {
                    def.frames.push_back(frame);
                }
            }
        }

        if (!def.baseTiles.empty() && !def.frames.empty()) {
            definitions.push_back(def);
        }
    }

    return definitions;
}

static json SerializeTileMap(const TileMap& tileMap) {
    json j = json::array();
    for (auto& [key, layerMap] : tileMap.GetAllTiles()) {
        int x = static_cast<int>(key >> 32);
        int y = static_cast<int>(key & 0xFFFFFFFF);

        // Iterate through all layers at this position
        for (auto const& [layerIdx, tile] : layerMap) {
            if (tile.tileID.empty()) continue;

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
    json j = json::array();

    for (const auto& entity : entities) {
        json entityJson;

        if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
            entityJson["type"] = "Player";
            entityJson["gridX"] = player->GetGridX();
            entityJson["gridY"] = player->GetGridY();
            entityJson["layer"] = player->GetEntityLayer();
            entityJson["scale"] = player->GetScale();
            if (player->GetAsset() != nullptr) {
                entityJson["assetID"] = player->GetAsset()->id;
            }
            j.push_back(entityJson);
        }
        else if (auto enemy = dynamic_cast<EnemyEntity*>(entity.get())) {
            entityJson["type"] = "Enemy";
            entityJson["gridX"] = enemy->GetGridX();
            entityJson["gridY"] = enemy->GetGridY();
            entityJson["layer"] = enemy->GetEntityLayer();
            entityJson["scale"] = enemy->GetScale();
            if (enemy->GetAsset() != nullptr) {
                entityJson["assetID"] = enemy->GetAsset()->id;
            }
            j.push_back(entityJson);
        }
        else if (auto staticEnt = dynamic_cast<StaticEntity*>(entity.get())) {
            entityJson["type"] = "Static";
            entityJson["gridX"] = staticEnt->GetGridX();
            entityJson["gridY"] = staticEnt->GetGridY();
            entityJson["layer"] = staticEnt->GetEntityLayer();
            entityJson["scale"] = staticEnt->GetScale();
            if (staticEnt->GetAsset() != nullptr) {
                entityJson["assetID"] = staticEnt->GetAsset()->id;
            }
            j.push_back(entityJson);
        }
    }

    return j;
}

bool SaveLoad::SaveProject(Engine& engine, const std::string& filename) {
    try {
        json projectRoot;
        projectRoot["version"] = "1.2";

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
        projectRoot["tileAnimations"] = SerializeAnimatedTileDefinitions(engine);

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

        if (projectRoot.contains("tileAnimations")) {
            engine.SetAnimatedTileDefinitions(DeserializeAnimatedTileDefinitions(projectRoot["tileAnimations"]));
        } else {
            engine.ClearAnimatedTileDefinitions();
        }
        engine.SetAnimatedTileClock(0.0f);

        // Load engine settings FIRST (before loading scenes)
        // This ensures layer count is set before we try to load tiles with layers
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
                        engine.SetLayerVisible(static_cast<int>(i), visibilityArray[i].get<bool>());
                    }
                }
            }
        }

        // Now clear and load scenes
        auto& scenes = engine.GetAllScenes();
        scenes.clear();

        for (const auto& scene : scenesJson) {
            std::string sceneId = scene.value("id", "");
            std::string sceneName = scene.value("name", "Unnamed Scene");

            auto newScene = std::make_unique<Scene>(sceneId, sceneName);

            // Load tiles
            if (scene.contains("tiles") && scene["tiles"].is_array()) {
                TileMap& newTileMap = newScene->GetTileMap();
                for (const auto& jsonTileMap : scene["tiles"]) {
                    int x = jsonTileMap.value("x", 0);
                    int y = jsonTileMap.value("y", 0);

                    // Get tile ID first to check if it's valid
                    std::string tileID = jsonTileMap.value("tileID", "");
                    if (tileID.empty()) {
                        // Skip empty tiles
                        continue;
                    }

                    TileData tileData;
                    tileData.tileID = tileID;
                    tileData.tileIndex = jsonTileMap.value("tileIndex", 0);
                    tileData.isSolid = jsonTileMap.value("isSolid", false);
                    tileData.isSceneSwitcher = jsonTileMap.value("isSceneSwitcher", false);
                    tileData.targetSceneID = jsonTileMap.value("targetSceneID", "");
                    tileData.triggerKey = jsonTileMap.value("triggerKey", 0);
                    tileData.layer = jsonTileMap.value("layer", 0);

                    newTileMap.SetTile(x, y, tileData, tileData.layer);
                }
            }

            // Load entities
            if (scene.contains("entities") && scene["entities"].is_array()) {
                auto& editModeEntities = newScene->GetEditModeEntities();
                for (const auto& jsonEntity : scene["entities"]) {
                    if (!jsonEntity.contains("type")) {
                        continue;
                    }

                    std::string entityType = jsonEntity["type"].get<std::string>();

                    if (entityType == "Player") {
                        int gridX = jsonEntity.value("gridX", 0);
                        int gridY = jsonEntity.value("gridY", 0);
                        int layer = jsonEntity.value("layer", 0);
                        float scale = jsonEntity.value("scale", 1.0f);
                        std::string assetID = jsonEntity.value("assetID", "");

                        Asset* asset = engine.GetAssetManager().GetAsset(assetID);

                        if (asset != nullptr) {
                            auto playerEntity = std::make_unique<PlayerEntity>(engine.GetGrid(), asset, gridX, gridY);
                            playerEntity->SetEntityLayer(layer);
                            playerEntity->SetScale(scale);
                            editModeEntities.push_back(std::move(playerEntity));
                        } else {
                            UI::SetDebugMessage("[WARNING] Player asset '" + assetID + "' not found. Skipping placement.");
                        }
                    }
                    else if (entityType == "Enemy") {
                        int gridX = jsonEntity.value("gridX", 0);
                        int gridY = jsonEntity.value("gridY", 0);
                        int layer = jsonEntity.value("layer", 0);
                        float scale = jsonEntity.value("scale", 1.0f);
                        std::string assetID = jsonEntity.value("assetID", "");

                        Asset* asset = engine.GetAssetManager().GetAsset(assetID);

                        if (asset != nullptr) {
                            auto enemyEntity = std::make_unique<EnemyEntity>(engine.GetGrid(), asset, gridX, gridY);
                            enemyEntity->SetEntityLayer(layer);
                            enemyEntity->SetScale(scale);
                            editModeEntities.push_back(std::move(enemyEntity));
                        }
                    }
                    else if (entityType == "Static") {
                        int gridX = jsonEntity.value("gridX", 0);
                        int gridY = jsonEntity.value("gridY", 0);
                        int layer = jsonEntity.value("layer", 0);
                        float scale = jsonEntity.value("scale", 1.0f);
                        std::string assetID = jsonEntity.value("assetID", "");

                        // Find the asset in AssetManager
                        Asset* asset = engine.GetAssetManager().GetAsset(assetID);
                        if (asset != nullptr) {
                            auto staticEntity = std::make_unique<StaticEntity>(
                                engine.GetGrid(), asset, gridX, gridY, layer
                            );
                            staticEntity->SetScale(scale);
                            staticEntity->SetEntityLayer(layer);
                            editModeEntities.push_back(std::move(staticEntity));
                        } else {
                            UI::SetDebugMessage("[WARNING] Asset not found: " + assetID + " - Static entity skipped");
                        }
                    }
                }
            }

            // Load camera areas
            if (scene.contains("editModeCameraArea")) {
                Rectangle editModeCameraArea = JsonToRect(scene["editModeCameraArea"]);
                newScene->SetEditModeCameraArea(editModeCameraArea);
            } else if (scene.contains("editModeCamera")) {
                Rectangle editModeCameraArea = JsonToRect(scene["editModeCamera"]);
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

        // Load remaining engine settings
        if (engineJson.contains("selectedResolutionIndex")) {
            engine.SetSelectedResolutionIndex(engineJson.value("selectedResolutionIndex", 1));
        }
        else {
            UI::SetDebugMessage("[WARNING] 'selectedResolutionIndex' not found in project file. Using default.");
        }

        if (engineJson.contains("currentSceneIndex")) {
            engine.SetCurrentSceneByIndex(engineJson.value("currentSceneIndex", 0));
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

        engine.StopPlayMode();
        engine.currentTool = Engine::ToolState::NONE;
        UI::SetDebugMessage("Successfully loaded project from " + filepath);

        return true;
    } catch (const std::exception& e) {
        UI::SetDebugMessage("[ERROR] Failed to load project: " + std::string(e.what()));
        return false;
    }
}
