// AssetDefinition.cpp - Simple, correct implementation
#include "AssetDefinition.h"
#include "UI/UI.h"
#include <fstream>

using json = nlohmann::json;

static std::string AssetTypeToString(AssetType type) {
    switch (type) {
        case AssetType::INDIVIDUAL_TEXTURE: return "INDIVIDUAL_TEXTURE";
        case AssetType::ANIMATED_SPRITESHEET: return "ANIMATED_SPRITESHEET";
        case AssetType::STATIC_SPRITESHEET: return "STATIC_SPRITESHEET";
        case AssetType::TILESET: return "TILESET";
        case AssetType::PLAYER: return "PLAYER";
        case AssetType::ENEMY: return "ENEMY";
        default: return "OTHER";
    }
}

static AssetType StringToAssetType(const std::string& typeStr) {
    if (typeStr == "INDIVIDUAL_TEXTURE") return AssetType::INDIVIDUAL_TEXTURE;
    if (typeStr == "ANIMATED_SPRITESHEET") return AssetType::ANIMATED_SPRITESHEET;
    if (typeStr == "STATIC_SPRITESHEET") return AssetType::STATIC_SPRITESHEET;
    if (typeStr == "TILESET") return AssetType::TILESET;
    if (typeStr == "PLAYER") return AssetType::PLAYER;
    if (typeStr == "ENEMY") return AssetType::ENEMY;
    return AssetType::OTHER;
}

namespace AssetPersistence {

json AssetDefinitionToJson(const AssetDefinition& def) {
    json j;
    j["id"] = def.id;
    j["name"] = def.name;
    j["category"] = def.category;
    j["path"] = def.path;
    j["type"] = AssetTypeToString(def.type);
    j["hasSelectedFrame"] = def.hasSelectedFrame;
    j["isTileset"] = def.isTileset;

    if (def.hasSelectedFrame) {
        j["frameWidth"] = def.frameWidth;
        j["frameHeight"] = def.frameHeight;
        j["selectedFrameX"] = def.selectedFrameX;
        j["selectedFrameY"] = def.selectedFrameY;
    }

    if (def.isTileset) {
        j["tileWidth"] = def.tileWidth;
        j["tileHeight"] = def.tileHeight;
    }

    j["hasAnimations"] = def.hasAnimations;
    if (def.hasAnimations && !def.animations.empty()) {
        json animArray = json::array();
        for (const auto& anim : def.animations) {
            json animJson;
            animJson["name"] = anim.name;
            animJson["sourceAssetId"] = anim.sourceAssetId;
            animJson["row"] = anim.row;
            animJson["frameCount"] = anim.frameCount;
            animJson["frameRate"] = anim.frameRate;
            animJson["loop"] = anim.loop;
            animJson["flipHorizontallyAtRuntime"] = anim.flipHorizontallyAtRuntime;
            animJson["trigger"] = anim.trigger;
            animJson["direction"] = anim.direction;
            animArray.push_back(animJson);
        }
        j["animations"] = animArray;
    }

    return j;
}

AssetDefinition JsonToAssetDefinition(const json& j) {
    AssetDefinition def;
    def.id = j.value("id", "");
    def.name = j.value("name", "");
    def.category = j.value("category", "");
    def.path = j.value("path", "");
    def.type = StringToAssetType(j.value("type", "OTHER"));
    def.isTileset = j.value("isTileset", false);
    def.hasSelectedFrame = j.value("hasSelectedFrame", false);
    def.frameWidth = j.value("frameWidth", 0);
    def.frameHeight = j.value("frameHeight", 0);
    def.selectedFrameX = j.value("selectedFrameX", 0);
    def.selectedFrameY = j.value("selectedFrameY", 0);
    def.tileWidth = j.value("tileWidth", 0);
    def.tileHeight = j.value("tileHeight", 0);

    // NEW: Deserialize animations
    def.hasAnimations = j.value("hasAnimations", false);
    if (def.hasAnimations && j.contains("animations")) {
        for (const auto& animJson : j["animations"]) {
            AnimationData anim;
            anim.name = animJson.value("name", "");
            anim.sourceAssetId = animJson.value("sourceAssetId", "");
            anim.row = animJson.value("row", 0);
            anim.frameCount = animJson.value("frameCount", 4);
            anim.frameRate = animJson.value("frameRate", 8.0f);
            anim.loop = animJson.value("loop", true);
            anim.trigger = static_cast<AnimationTrigger>(animJson.value("trigger", 0));
            anim.flipHorizontallyAtRuntime = animJson.value("flipHorizontallyAtRuntime", false);
            anim.direction = static_cast<AnimationDirection>(animJson.value("direction", 0));
            def.animations.push_back(anim);
        }
    }

    return def;
}

bool SaveAssetDefinitions(const AssetManager& assetManager, const std::string& filepath) {
    try {
        json root;
        root["version"] = "1.0";
        root["assets"] = json::array();

        const auto& allAssets = assetManager.GetAllAssets();

        for (const auto* asset : allAssets) {
            AssetDefinition def;
            def.id = asset->id;
            def.name = asset->name;
            def.category = asset->category;
            def.path = asset->path;
            def.type = asset->type;
            def.isTileset = false;
            def.hasSelectedFrame = false;

            // Check what kind of asset this is
            if (!asset->subSprites.empty() && asset->type == AssetType::TILESET) {
                def.isTileset = true;
                if (asset->SpriteSize.x > 0 && asset->SpriteSize.y > 0) {
                    def.tileWidth = static_cast<int>(asset->SpriteSize.x);
                    def.tileHeight = static_cast<int>(asset->SpriteSize.y);
                }
            }
            else if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0 &&
                     asset->SpriteSize.x > 0 && asset->SpriteSize.y > 0) {
                def.hasSelectedFrame = true;
                def.frameWidth = static_cast<int>(asset->SpriteSize.x);
                def.frameHeight = static_cast<int>(asset->SpriteSize.y);
                def.selectedFrameX = static_cast<int>(asset->SpriteSourceRect.x / asset->SpriteSize.x);
                def.selectedFrameY = static_cast<int>(asset->SpriteSourceRect.y / asset->SpriteSize.y);
            }

            // NEW: Save animations if they exist
            if (asset->HasAnimations() && asset->animationSet) {
                def.hasAnimations = true;
                def.frameWidth = asset->animationSet->frameWidth;
                def.frameHeight = asset->animationSet->frameHeight;

                // Convert AnimationSet animations to AnimationData for persistence
                for (const auto& [name, anim] : asset->animationSet->animations) {
                    AnimationData animData;
                    animData.name = name;
                    animData.sourceAssetId = anim.sourceAssetId.empty() ? asset->id : anim.sourceAssetId;
                    animData.loop = anim.loop;
                    animData.trigger = anim.trigger;
                    animData.direction = anim.direction;
                    animData.flipHorizontallyAtRuntime = anim.flipHorizontallyAtRuntime;

                    // Extract row and frame count from first frame
                    if (!anim.frames.empty()) {
                        const auto& firstFrame = anim.frames[0];
                        animData.row = static_cast<int>(firstFrame.sourceRect.y / asset->animationSet->frameHeight);
                        animData.frameCount = static_cast<int>(anim.frames.size());

                        if (firstFrame.duration > 0.0f) {
                            animData.frameRate = 1.0f / firstFrame.duration;
                        }
                    }

                    def.animations.push_back(animData);
                }
            }

            root["assets"].push_back(AssetDefinitionToJson(def));
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            UI::SetDebugMessage("[ASSET PERSISTENCE] Failed to open file for writing: " + filepath);
            return false;
        }

        file << root.dump(4);
        file.close();

        UI::SetDebugMessage("[ASSET PERSISTENCE] Saved " + std::to_string(allAssets.size()) + " asset definitions");
        return true;

    } catch (const std::exception& e) {
        UI::SetDebugMessage("[ASSET PERSISTENCE] Error saving: " + std::string(e.what()));
        return false;
    }
}

bool LoadAssetDefinitions(AssetManager& assetManager, const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            UI::SetDebugMessage("[ASSET PERSISTENCE] Asset definitions file not found: " + filepath);
            return false;
        }

        json root;
        file >> root;
        file.close();

        if (!root.contains("assets")) {
            UI::SetDebugMessage("[ASSET PERSISTENCE] Invalid asset definitions file");
            return false;
        }

        int loadedCount = 0;
        const auto& assetsArray = root["assets"];

        for (const auto& assetJson : assetsArray) {
            AssetDefinition def = JsonToAssetDefinition(assetJson);

            if (assetManager.GetAsset(def.id) != nullptr) {
                continue;
            }

            if (def.isTileset) {
                assetManager.LoadAssetWithType(
                    def.id, def.name, def.category, def.path,
                    def.type, def.tileWidth, def.tileHeight
                );
                loadedCount++;
            } else if (def.hasSelectedFrame) {
                assetManager.LoadAssetWithFrame(
                    def.id, def.name, def.category, def.path,
                    def.frameWidth, def.frameHeight,
                    def.selectedFrameX, def.selectedFrameY,
                    def.type
                );
                loadedCount++;
            } else {
                assetManager.LoadAssetWithType(
                    def.id, def.name, def.category, def.path, def.type
                );
                loadedCount++;
            }

            Asset* asset = assetManager.GetAsset(def.id);
            if (asset && def.hasAnimations && !def.animations.empty()) {
                // Create AnimationSet
                if (!asset->animationSet) {
                    asset->animationSet = std::make_unique<AnimationSet>();
                }

                AnimationSet* animSet = asset->animationSet.get();
                animSet->animations.clear();
                animSet->id = asset->id + "_animations";
                animSet->textureId = asset->id;
                animSet->texture = asset->texture;
                animSet->textureLoaded = asset->loaded;
                animSet->frameWidth = def.frameWidth;
                animSet->frameHeight = def.frameHeight;

                // Rebuild animations from saved data
                for (const auto& animData : def.animations) {
                    animSet->AddAnimationFromRow(
                        animData.name,
                        animData.row,
                        animData.frameCount,
                        animData.frameRate,
                        animData.loop,
                        animData.flipHorizontallyAtRuntime,
                        animData.trigger,
                        animData.direction,
                        animData.sourceAssetId.empty() ? asset->id : animData.sourceAssetId
                        );

                    Animation* loadedAnim = animSet->GetAnimation(animData.name);
                    if (loadedAnim) {
                        Asset* sourceAsset = assetManager.GetAsset(loadedAnim->sourceAssetId);
                        if (sourceAsset && sourceAsset->loaded) {
                            loadedAnim->sourceTexture = sourceAsset->texture;
                            loadedAnim->hasSourceTexture = true;
                        } else {
                            loadedAnim->sourceTexture = asset->texture;
                            loadedAnim->hasSourceTexture = true;
                        }
                    }
                }
                asset->hasAnimations = true;
            }
        }

        // Second pass: resolve cross-asset animation textures when source asset appears later in file.
        for (auto* asset : assetManager.GetAllAssets()) {
            if (!asset || !asset->HasAnimations() || !asset->animationSet) continue;
            for (auto& [name, anim] : asset->animationSet->animations) {
                const std::string sourceId = anim.sourceAssetId.empty() ? asset->id : anim.sourceAssetId;
                Asset* sourceAsset = assetManager.GetAsset(sourceId);
                if (sourceAsset && sourceAsset->loaded) {
                    anim.sourceTexture = sourceAsset->texture;
                    anim.hasSourceTexture = true;
                }
            }
        }

        UI::SetDebugMessage("[ASSET PERSISTENCE] Loaded " + std::to_string(loadedCount) + " assets");
        return true;

    } catch (const std::exception& e) {
        UI::SetDebugMessage("[ASSET PERSISTENCE] Error loading: " + std::string(e.what()));
        return false;
    }
}
}
