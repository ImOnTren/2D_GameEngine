#include "AssetManager.h"
#include "UI/UI.h"
#include <algorithm>
#include <filesystem>

namespace {
    std::string NormalizeLegacyAssetPath(const std::string& rawPath) {
        std::string normalized = rawPath;
        std::replace(normalized.begin(), normalized.end(), '\\', '/');

        const std::string legacyPrefix = "../src/assets/";
        if (normalized.rfind(legacyPrefix, 0) == 0) {
            normalized = "assets/" + normalized.substr(legacyPrefix.size());
        }
        return normalized;
    }

    std::string ResolveAssetPath(const std::string& rawPath) {
        const std::string normalized = NormalizeLegacyAssetPath(rawPath);

        if (FileExists(normalized.c_str())) {
            return normalized;
        }

        // Support cases where process working directory is one level above.
        const std::filesystem::path alt = std::filesystem::path("..") / normalized;
        const std::string altString = alt.generic_string();
        if (FileExists(altString.c_str())) {
            return altString;
        }

        return normalized;
    }
}

AssetManager::AssetManager()
{

}

AssetManager::~AssetManager()
{

}

void AssetManager::LoadTextureForAsset(Asset* asset) {
    const std::string resolvedPath = ResolveAssetPath(asset->path);

    if (FileExists(resolvedPath.c_str()))
    {
        asset->texture = LoadTexture(resolvedPath.c_str());
        asset->loaded = true;
        asset->path = resolvedPath;

        // pixel-perfect sampling for tiles/sprites
        SetTextureFilter(asset->texture, TEXTURE_FILTER_POINT);
        // avoid wrapping at edges
        SetTextureWrap(asset->texture, TEXTURE_WRAP_CLAMP);

    }
    else
    {
        UI::SetDebugMessage("[ASSET] Failed to load texture: " + resolvedPath);
        asset->loaded = false;
    }
}

void AssetManager::LoadAssetWithFrame(const std::string& id,
                                     const std::string& name,
                                     const std::string& category,
                                     const std::string& path,
                                     int frameWidth,
                                     int frameHeight,
                                     int selectedFrameX,
                                     int selectedFrameY,
                                     AssetType type) {
    if (assets.find(id) != assets.end()) {
        UI::SetDebugMessage("[ASSET] Asset with id " + id + " already exists");
        return;
    }

    auto asset = std::make_unique<Asset>(id, name, category, path, type);

    // Load the texture
    LoadTextureForAsset(asset.get());

    if (asset->loaded) {
        // Store frame dimensions
        asset->SpriteSize = {static_cast<float>(frameWidth), static_cast<float>(frameHeight)};

        // Calculate and store the specific frame rectangle
        // This is the ONLY frame we'll use for display/placement
        asset->SpriteSourceRect = {
            static_cast<float>(selectedFrameX * frameWidth),
            static_cast<float>(selectedFrameY * frameHeight),
            static_cast<float>(frameWidth),
            static_cast<float>(frameHeight)
        };
        asset->hasSelectedFrame = true;

        // subSprites remains empty

        UI::SetDebugMessage("[ASSET] Loaded asset with selected frame (" +
                          std::to_string(selectedFrameX) + ", " +
                          std::to_string(selectedFrameY) + "): " + id);
    }

    assets[id] = std::move(asset);
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

std::vector<Asset*> AssetManager::GetAllAssets() const{
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

    //Small inset to avoid sampling outside tile borders
    const float eps = 0.01f;

    int columns = asset->texture.width / tileWidth;
    int rows = asset->texture.height / tileHeight;

    asset->subSprites.clear();
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            Rectangle rect;
            rect.x      = static_cast<float>(x * tileWidth);
            rect.y      = static_cast<float>(y * tileHeight);
            rect.width  = static_cast<float>(tileWidth) - 2.0f * eps;
            rect.height = static_cast<float>(tileHeight) - 2.0f * eps;
            asset->subSprites.push_back(rect);
        }
    }
    UI::SetDebugMessage("[ASSET] Processed tileset into " + std::to_string(asset->subSprites.size()) + " sub-sprites");
}

Rectangle AssetManager::GetSpecificSprite(const Asset* asset, const int& index){
    if (!asset->loaded) {
        UI::SetDebugMessage("[ASSET] Cannot get sprite: Asset not loaded");
        return {0, 0, 0, 0};
    }

    // Case 1: Asset is a TILESET with subSprites (hardcoded or imported tilesets)
    if (asset->type == AssetType::TILESET && !asset->subSprites.empty()) {
        if (index < 0 || index >= static_cast<int>(asset->subSprites.size())) {
            UI::SetDebugMessage("[ASSET] Invalid sprite index: " + std::to_string(index));
            return {0, 0, 0, 0};
        }
        return asset->subSprites[index];
    }

    // Case 2: Asset has a selected frame (imported animated/static assets)
    // Use this ONLY for display purposes (thumbnails, placement)
    // NOT for tile selection
    if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
        return asset->SpriteSourceRect;
    }

    // Case 3: Individual texture (full image)
    if (asset->type == AssetType::INDIVIDUAL_TEXTURE) {
        if (asset->SpriteSourceRect.width > 0 && asset->SpriteSourceRect.height > 0) {
            return asset->SpriteSourceRect;
        }
        return {0, 0, static_cast<float>(asset->texture.width), static_cast<float>(asset->texture.height)};
    }

    // Case 4: Fallback - return full texture
    return {0, 0, static_cast<float>(asset->texture.width), static_cast<float>(asset->texture.height)};
}

void AssetManager::LoadAsset(const std::string& id, const std::string& name,
                             const std::string& category, const std::string& path,
                             int spriteWidth, int spriteHeight,
                             const std::unordered_map<std::string, SpriteAnimation>& animation){
    const AssetType detectedType = DetectAssetType(name, category);
    LoadAssetWithType(id, name, category, path, detectedType, spriteWidth, spriteHeight, animation);
}

void AssetManager::LoadAssetWithType(const std::string& id, const std::string& name,
                             const std::string& category, const std::string& path,
                             const AssetType type, int spriteWidth, int spriteHeight,
                             const std::unordered_map<std::string, SpriteAnimation>& animation){
    if (assets.find(id) != assets.end())
    {
        UI::SetDebugMessage("[ASSET] Asset with id " + id + " already exists");
        return;
    }

    auto asset = std::make_unique<Asset>(id, name, category, path, type);

    LoadTextureForAsset(asset.get());

    asset->SpriteSourceRect = {0, 0, 0, 0};

    if (spriteWidth > 0 && spriteHeight > 0){
        asset->SpriteSize = {static_cast<float>(spriteWidth), static_cast<float>(spriteHeight)};
        if (type == AssetType::TILESET) {
            ProcessTileset(asset.get(), spriteHeight, spriteWidth);
        }
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

    if (nameLower.find("texture") != std::string::npos)
    {
        return AssetType::INDIVIDUAL_TEXTURE;
    }
    if (nameLower.find("sprite") != std::string::npos)
    {
        return AssetType::STATIC_SPRITESHEET;
    }
    if (nameLower.find("animation") != std::string::npos)
    {
        return AssetType::ANIMATED_SPRITESHEET;
    }
    if (nameLower.find("tileset") != std::string::npos || categoryLower == "tileset")
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
