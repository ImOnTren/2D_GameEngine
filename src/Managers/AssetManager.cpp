#include "AssetManager.h"
#include "UI/UI.h"
#include <algorithm>

AssetManager::AssetManager()
{

}

AssetManager::~AssetManager()
{

}

void AssetManager::LoadTextureForAsset(Asset* asset) {
    if (FileExists(asset->path.c_str()))
    {
        asset->texture = LoadTexture(asset->path.c_str());
        asset->loaded = true;
    }
    else
    {
        UI::SetDebugMessage("[ASSET] Failed to load texture: " + asset->path);
        asset->loaded = false;
    }
}

Asset* AssetManager::GetAsset(const std::string& id){
    if (assets.find(id) != assets.end())
    {
        return assets[id].get();
    }
    return nullptr;
}

std::vector<Asset*> AssetManager::GetAssetByCategory(const std::string& category){
    std::vector<Asset*> foundAssets;
    for (auto& [id, asset] : assets) {
        if (asset->category == category)
        {
            foundAssets.push_back(asset.get());
        }
    }
    return foundAssets;
}

std::vector<Asset*> AssetManager::GetAssetByType(const AssetType& type){
    std::vector<Asset*> foundAssets;
    for (auto& [id, asset] : assets) {
        if (asset->type == type)
        {
            foundAssets.push_back(asset.get());
        }
    }
    return foundAssets;
}

const std::vector<Asset*> AssetManager::GetAllAssets(){
    std::vector<Asset*> allAssets;
    for (auto& [id, asset] : assets) {
        allAssets.push_back(asset.get());
    }
    return allAssets;
}

const std::vector<std::string> AssetManager::GetCategories(){
    std::vector<std::string> categories;
    for (auto& [id, asset] : assets) {
        if (std::find(categories.begin(), categories.end(), asset->category) == categories.end())
        {
            categories.push_back(asset->category);
        }
    }
    return categories;
}

int AssetManager::GetAssetCount(){
    return assets.size();
}

void AssetManager::ProcessTileset(Asset* asset, int tileHeight, int tileWidth){
    if (!asset->loaded)
    {
        UI::SetDebugMessage("[ASSET] Cannot process tileset: Asset not loaded");
        return;
    }

    int columns = asset->texture.width / tileWidth;
    int rows = asset->texture.height / tileHeight;

    asset->subSprites.clear();
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            Rectangle rect;
            rect.x = static_cast<float>(x * tileWidth);
            rect.y = static_cast<float>(y * tileHeight);
            rect.width = static_cast<float>(tileWidth);
            rect.height = static_cast<float>(tileHeight);
            asset->subSprites.push_back(rect);
        }
    }
    UI::SetDebugMessage("[ASSET] Processed tileset into " + std::to_string(asset->subSprites.size()) + " sub-sprites");
}

Rectangle AssetManager::GetSpecificSprite(const Asset* asset, const int& index){
    if (!asset->loaded)
    {
        UI::SetDebugMessage("[ASSET] Cannot get sprite: Asset not loaded");
        return {0, 0, 0, 0};
    }
    if (index < 0 || index >= static_cast<int>(asset->subSprites.size()))
    {
        UI::SetDebugMessage("[ASSET] Invalid sprite index: " + std::to_string(index));
        return {0, 0, 0, 0};
    }
    if (asset->type == AssetType::INDIVIDUAL_TEXTURE){
        return asset->SpriteSourceRect;
    }

    if (index >= 0 && index < static_cast<int>(asset->subSprites.size()))
    {
        return asset->subSprites[index];
    }
    return asset->subSprites[0];
}

void AssetManager::LoadAsset(const std::string& id, const std::string& name,
                             const std::string& category, const std::string& path,
                             int spriteWidth, int spriteHeight,
                             const std::unordered_map<std::string, SpriteAnimation>& animation){
    if (assets.find(id) != assets.end())
    {
        UI::SetDebugMessage("[ASSET] Asset with id " + id + " already exists");
        return;
    }
    AssetType type = DetectAssetType(name, category);

    auto asset = std::make_unique<Asset>(id, name, category, path, type);

    LoadTextureForAsset(asset.get());

    if (spriteWidth > 0 && spriteHeight > 0){
        asset->SpriteSize = {static_cast<float>(spriteWidth), static_cast<float>(spriteHeight)};
        ProcessTileset(asset.get(), spriteHeight, spriteWidth);
    }

    assets[id] = std::move(asset);
    UI::SetDebugMessage("[ASSET] Loaded asset: " + id);
}

AssetType AssetManager::DetectAssetType(const std::string& name, const std::string& category)
{
    std::string nameLower = name;
    std::string categoryLower = category;
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
    std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::tolower);

    if (name.find("texture") != std::string::npos)
    {
        return AssetType::INDIVIDUAL_TEXTURE;
    }
    if (name.find("sprite") != std::string::npos)
    {
        return AssetType::STATIC_SPRITESHEET;
    }
    if (name.find("animation") != std::string::npos)
    {
        return AssetType::ANIMATED_SPRITESHEET;
    }
    if (name.find("tileset") != std::string::npos || categoryLower == "tileset")
    {
        return AssetType::TILESET;
    }
    return AssetType::OTHER;
}

void AssetManager::UnloadAllAssets()
{
    for(auto& [id, asset] : assets) {
        if (asset->loaded)
        {
            UnloadTexture(asset->texture);
        }
    }
    assets.clear();
    UI::SetDebugMessage("[ASSET] Unloaded all assets");
}