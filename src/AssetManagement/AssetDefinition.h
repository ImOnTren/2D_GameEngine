#pragma once

#include <string>
#include <vector>
#include "AssetManagement//AssetManager.h"
#include "nlohmann/json.hpp"

// Serializable animation data for persistence
struct AnimationData {
    std::string name;
    int row = 0;
    int frameCount = 4;
    float frameRate = 8.0f;
    bool loop = true;
    AnimationTrigger trigger = AnimationTrigger::LOOP;
    AnimationDirection direction = AnimationDirection::NONE;
};

struct AssetDefinition {
    std::string id;
    std::string name;
    std::string category;
    std::string path;
    AssetType type;

    // Frame/Sprite info
    int frameWidth = 0;
    int frameHeight = 0;
    int selectedFrameX = 0;
    int selectedFrameY = 0;
    bool hasSelectedFrame = false;

    // Tileset specific
    int tileWidth = 0;
    int tileHeight = 0;
    bool isTileset = false;

    // Animation data
    bool hasAnimations = false;
    std::vector<AnimationData> animations;
};

namespace AssetPersistence {
    bool SaveAssetDefinitions(const AssetManager& assetManager, const std::string& filepath);
    bool LoadAssetDefinitions(AssetManager& assetManager, const std::string& filepath);
    nlohmann::json AssetDefinitionToJson(const AssetDefinition& def);
    AssetDefinition JsonToAssetDefinition(const nlohmann::json& j);
}