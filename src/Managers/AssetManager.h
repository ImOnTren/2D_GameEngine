#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "raylib.h"

enum class AssetType {
    INDIVIDUAL_TEXTURE,
    ANIMATED_SPRITESHEET,
    STATIC_SPRITESHEET,
    TILESET,
    PLAYER,
    ENEMY,
    GRASS_BLOCK,
    STONE_BLOCK,
    TREE,
    HOUSE,
    CAR,
    ANIMAL,
    UI_ELEMENT,
    FONT,
    SOUND,
    MUSIC,
    OTHER
};

struct SpriteFrame {
    Rectangle sourceRect;
    float duration;
};

struct SpriteAnimation {
    std::vector<SpriteFrame> frames;
    bool loop;
    int frameRate;
};

struct Asset {
    std::string id;
    std::string name;
    AssetType type;
    std::string category;
    std::string path;
    Texture2D texture;
    bool loaded;

    Vector2 SpriteSize;
    Rectangle SpriteSourceRect;
    std::vector<Rectangle> subSprites;

    Asset(std::string id, std::string name, std::string category, std::string path, AssetType type)
        : id(id), name(name), path(path), type(type), category(category), loaded(false){}
};

class AssetManager{
    public:
    AssetManager();
    ~AssetManager();

    AssetType DetectAssetType(const std::string& name, const std::string& category);
    void LoadAsset(const std::string& id, const std::string& name, const std::string& category, const std::string& path,
                   int spriteWidth = 0, int spriteHeight = 0,
                   const std::unordered_map<std::string, SpriteAnimation>& animation = {});
    void LoadTextureForAsset(Asset* asset);
    void UnloadAllAssets();

    Asset* GetAsset(const std::string& id);
    std::vector<Asset*> GetAssetByCategory(const std::string& category);
    std::vector<Asset*> GetAssetByType(const AssetType& type);
    const std::vector<Asset*> GetAllAssets();
    const std::vector<std::string> GetCategories();
    int GetAssetCount();

    Rectangle GetSpecificSprite(const Asset* asset, const int& index);

    void ProcessTileset(Asset* asset, int tileHeight, int tileWidth);

    private:
    std::unordered_map<std::string, std::unique_ptr<Asset>> assets;
};