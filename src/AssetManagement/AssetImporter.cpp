// AssetImporter.cpp - COMPLETE FILE with preview fix integrated
// Replace your entire AssetImporter.cpp with this

#include "AssetImporter.h"
#include "Engine.h"
#include "imgui.h"
#include "UI/UI.h"
#include "../Managers/LocalizationManager.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

#define TR(key) Localization::Get(key)

AssetImporter::~AssetImporter() {
    UnloadCurrentTexture();
}

void AssetImporter::Render(Engine& engine) {
    if (!windowOpen) return;

    if (needsRescan) {
        ScanForPngFiles(searchPathBuffer);
        needsRescan = false;
    }

    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 50), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(TR("importer.window_title"), &windowOpen)) {

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", TR("importer.subtitle"));
        ImGui::Separator();

        ImGui::BeginChild("LeftPanel", ImVec2(400, 0), true);

        RenderFileBrowser();

        if (textureLoaded) {
            ImGui::Separator();
            RenderFrameSelector();
            ImGui::Separator();
            RenderAssetParameters();
            ImGui::Separator();
            RenderCreateButton(engine);
        }

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
        RenderTexturePreview();
        ImGui::EndChild();
    }
    ImGui::End();
}

void AssetImporter::RenderFileBrowser() {
    ImGui::Text("%s", TR("importer.step1"));
    ImGui::Spacing();

    ImGui::Text("%s", TR("importer.search_directory"));
    ImGui::SetNextItemWidth(-80);
    ImGui::InputText("##SearchPath", searchPathBuffer, sizeof(searchPathBuffer));
    ImGui::SameLine();
    if (ImGui::Button(TR("importer.scan"), ImVec2(70, 0))) {
        ScanForPngFiles(searchPathBuffer);
    }

    ImGui::Text(TR("importer.found_png_files"), (int)foundPngFiles.size());
    ImGui::Spacing();

    ImGui::BeginChild("FileList", ImVec2(0, 200), true);
    for (int i = 0; i < (int)foundPngFiles.size(); i++) {
        ImGui::PushID(i);

        bool isSelected = (selectedFileIndex == i);
        if (ImGui::Selectable(foundPngFiles[i].displayPath.c_str(), isSelected)) {
            selectedFileIndex = i;
            LoadTexture(foundPngFiles[i].fullPath);

            std::string filename = foundPngFiles[i].filename;
            size_t dotPos = filename.find_last_of('.');
            if (dotPos != std::string::npos) {
                filename = filename.substr(0, dotPos);
            }
            strncpy(assetNameBuffer, filename.c_str(), sizeof(assetNameBuffer) - 1);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", foundPngFiles[i].fullPath.c_str());
        }

        ImGui::PopID();
    }
    ImGui::EndChild();

    if (textureLoaded) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), TR("importer.loaded_path"), currentTexturePath.c_str());
        ImGui::Text(TR("importer.size"), currentTexture.width, currentTexture.height);
    }
}

void AssetImporter::RenderTexturePreview() {
    ImGui::Text("%s", TR("importer.texture_preview"));
    ImGui::Separator();

    if (!textureLoaded) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", TR("importer.select_png_to_preview"));
        return;
    }

    // DEBUG INFO
    /*ImGui::Text("DEBUG INFO:");
    ImGui::Text("Selected Asset Type Index: %d", selectedAssetType);
    if (selectedAssetType >= 0 && selectedAssetType < assetTypes.size()) {
        ImGui::Text("Asset Type Name: %s", assetTypes[selectedAssetType].key);
    }
    ImGui::Text("Frame Selected: %s", frameSelected ? "TRUE" : "FALSE");
    ImGui::Text("Frame Size: %dx%d", frameWidth, frameHeight);
    ImGui::Text("Selected Frame: (%d, %d)", selectedFrameX, selectedFrameY);
    ImGui::Separator(); */
    ImGui::Text(TR("importer.texture_size"), currentTexture.width, currentTexture.height);

    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    availableSize.y -= 150;  // More space for debug info

    // Determine what to show
    AssetType selectedType = assetTypes[selectedAssetType].type;

    /*ImGui::Text("Checking conditions:");
    ImGui::Text("Is ANIMATED/PLAYER/ENEMY? %s",
               (selectedType == AssetType::ANIMATED_SPRITESHEET ||
                selectedType == AssetType::PLAYER ||
                selectedType == AssetType::ENEMY) ? "YES" : "NO");
    ImGui::Text("Frame selected? %s", frameSelected ? "YES" : "NO");
    ImGui::Text("Frame smaller than texture? %s",
               (frameWidth < currentTexture.width || frameHeight < currentTexture.height) ? "YES" : "NO");*/

    bool showSelectedFrameOnly = false;

    if ((selectedType == AssetType::ANIMATED_SPRITESHEET ||
         selectedType == AssetType::PLAYER ||
         selectedType == AssetType::ENEMY ||
         selectedType == AssetType::STATIC_SPRITESHEET) &&
        frameSelected &&
        frameWidth > 0 && frameHeight > 0 &&
        (frameWidth < currentTexture.width || frameHeight < currentTexture.height)) {
        showSelectedFrameOnly = true;
    }

    //ImGui::Text("Show Selected Frame Only? %s", showSelectedFrameOnly ? "YES" : "NO");
    //ImGui::Separator();

    if (showSelectedFrameOnly) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", TR("importer.showing_selected_frame_only"));

        float srcX = static_cast<float>(selectedFrameX * frameWidth);
        float srcY = static_cast<float>(selectedFrameY * frameHeight);

        ImVec2 uv0(srcX / currentTexture.width, srcY / currentTexture.height);
        ImVec2 uv1((srcX + frameWidth) / currentTexture.width, (srcY + frameHeight) / currentTexture.height);

        ImGui::Text(TR("importer.uv0"), uv0.x, uv0.y);
        ImGui::Text(TR("importer.uv1"), uv1.x, uv1.y);

        float scale = std::min(availableSize.x / frameWidth, availableSize.y / frameHeight);
        scale = std::max(scale, 1.0f);
        scale = std::min(scale, 8.0f);

        float displayWidth = frameWidth * scale;
        float displayHeight = frameHeight * scale;

        ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        ImGui::Image((ImTextureID)(intptr_t)currentTexture.id,
                     ImVec2(displayWidth, displayHeight), uv0, uv1);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(cursorPos,
                         ImVec2(cursorPos.x + displayWidth, cursorPos.y + displayHeight),
                         IM_COL32(0, 255, 0, 255), 0.0f, 0, 3.0f);

    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", TR("importer.showing_full_texture"));

        float scale = std::min(availableSize.x / currentTexture.width,
                              availableSize.y / currentTexture.height);
        scale = std::max(scale, 0.5f);
        scale = std::min(scale, 8.0f);

        float displayWidth = currentTexture.width * scale;
        float displayHeight = currentTexture.height * scale;

        ImGui::Image((ImTextureID)(intptr_t)currentTexture.id,
                     ImVec2(displayWidth, displayHeight));
    }
}

void AssetImporter::RenderFrameSelector() {
    ImGui::Text("%s", TR("importer.step2"));
    ImGui::Spacing();

    AssetType selectedType = assetTypes[selectedAssetType].type;

    if (selectedType == AssetType::TILESET) {
        ImGui::TextWrapped("%s", TR("importer.tileset_hint_line1"));
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                          "%s", TR("importer.tileset_hint_line2"));
    } else if (selectedType == AssetType::ANIMATED_SPRITESHEET ||
               selectedType == AssetType::PLAYER ||
               selectedType == AssetType::ENEMY) {
        ImGui::TextWrapped("%s", TR("importer.animated_hint_line1"));
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
                          "%s", TR("importer.animated_hint_line2"));
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                          "%s", TR("importer.animated_hint_line3"));
    } else if (selectedType == AssetType::STATIC_SPRITESHEET) {
        ImGui::TextWrapped("%s", TR("importer.static_hint_line1"));
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                          "%s", TR("importer.static_hint_line2"));
    } else {
        ImGui::TextWrapped("%s", TR("importer.individual_hint_line1"));
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                          "%s", TR("importer.individual_hint_line2"));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    bool changed = false;

    ImGui::Text("%s", TR("importer.frame_size"));
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputInt("Width##Frame", &frameWidth, 8, 16)) {
        if (frameWidth < 1) frameWidth = 1;
        if (frameWidth > currentTexture.width) frameWidth = currentTexture.width;
        changed = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputInt("Height##Frame", &frameHeight, 8, 16)) {
        if (frameHeight < 1) frameHeight = 1;
        if (frameHeight > currentTexture.height) frameHeight = currentTexture.height;
        changed = true;
    }

    ImGui::Text("%s", TR("importer.presets"));
    if (ImGui::Button(TR("importer.full_image"))) {
        frameWidth = currentTexture.width;
        frameHeight = currentTexture.height;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("16x16")) { frameWidth = 16; frameHeight = 16; changed = true; }
    ImGui::SameLine();
    if (ImGui::Button("32x32")) { frameWidth = 32; frameHeight = 32; changed = true; }
    ImGui::SameLine();
    if (ImGui::Button("48x48")) { frameWidth = 48; frameHeight = 48; changed = true; }
    if (ImGui::Button("64x64")) { frameWidth = 64; frameHeight = 64; changed = true; }

    if (changed) {
        RecalculateGridSize();
        selectedFrameX = 0;
        selectedFrameY = 0;
        frameSelected = true;
    }

    if (columnsInSheet > 0 && rowsInSheet > 0) {
        ImGui::Spacing();
        ImGui::Text(TR("importer.spritesheet_grid"), columnsInSheet, rowsInSheet);

        if (frameWidth < currentTexture.width || frameHeight < currentTexture.height) {
            ImGui::Spacing();

            if (selectedType == AssetType::TILESET) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                   TR("importer.all_tiles_available"), columnsInSheet * rowsInSheet);
            } else {
                ImGui::Text("%s", TR("importer.select_frame_placeholder"));
                ImGui::SetNextItemWidth(150);
                if (ImGui::SliderInt("Column##FrameX", &selectedFrameX, 0, columnsInSheet - 1)) {
                    frameSelected = true;
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                if (ImGui::SliderInt("Row##FrameY", &selectedFrameY, 0, rowsInSheet - 1)) {
                    frameSelected = true;
                }

                ImGui::Checkbox(TR("importer.use_selected_frame"), &frameSelected);

                if (frameSelected) {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                                       TR("importer.selected_frame_info"),
                                      selectedFrameX, selectedFrameY,
                                      selectedFrameY * columnsInSheet + selectedFrameX);
                }
            }
        }
    }
}

void AssetImporter::RenderAssetParameters() {
    ImGui::Text("%s", TR("importer.step3"));
    ImGui::Spacing();

    ImGui::Text("%s", TR("importer.asset_name"));
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##AssetName", assetNameBuffer, sizeof(assetNameBuffer));

    ImGui::Text("%s", TR("importer.asset_id"));
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##AssetID", assetIdBuffer, sizeof(assetIdBuffer));
    ImGui::SameLine();
    if (ImGui::Button(TR("importer.auto"))) {
        std::string baseName = assetNameBuffer;
        std::transform(baseName.begin(), baseName.end(), baseName.begin(), ::tolower);
        std::replace(baseName.begin(), baseName.end(), ' ', '_');
        strncpy(assetIdBuffer, baseName.c_str(), sizeof(assetIdBuffer) - 1);
    }

    ImGui::Text("%s", TR("category"));
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##Category", assetCategoryBuffer, sizeof(assetCategoryBuffer));

    ImGui::Text("%s", TR("importer.asset_type"));
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##AssetType", TR(assetTypes[selectedAssetType].key))) {
        for (int i = 0; i < (int)assetTypes.size(); i++) {
            bool isSelected = (selectedAssetType == i);
            if (ImGui::Selectable(TR(assetTypes[i].key), isSelected)) {
                selectedAssetType = i;
                isTileset = (assetTypes[i].type == AssetType::TILESET);
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (isTileset) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", TR("importer.tileset_settings"));

        ImGui::Text("%s", TR("tile_size"));
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Tile Width##Tileset", &tilesetTileWidth, 1, 8);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Tile Height##Tileset", &tilesetTileHeight, 1, 8);

        if (tilesetTileWidth < 1) tilesetTileWidth = 1;
        if (tilesetTileHeight < 1) tilesetTileHeight = 1;
    }
}

void AssetImporter::RenderCreateButton(Engine& engine) {
    ImGui::Spacing();
    ImGui::Separator();

    bool canCreate = true;
    std::string errorMsg;

    if (strlen(assetNameBuffer) == 0) {
        canCreate = false;
        errorMsg = TR("importer.error_asset_name_required");
    } else if (strlen(assetIdBuffer) == 0) {
        canCreate = false;
        errorMsg = TR("importer.error_asset_id_required");
    } else if (!textureLoaded) {
        canCreate = false;
        errorMsg = TR("importer.error_no_texture_loaded");
    }

    if (!canCreate) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", errorMsg.c_str());
    }

    ImGui::BeginDisabled(!canCreate);
    if (ImGui::Button(TR("importer.create_asset"), ImVec2(-1, 40))) {
        CreateAsset(engine);
    }
    ImGui::EndDisabled();
}

void AssetImporter::CreateAsset(Engine& engine) {
    AssetManager& assetManager = engine.GetAssetManager();

    std::string assetId = GenerateUniqueId(assetIdBuffer, assetManager);
    std::string assetName = assetNameBuffer;
    std::string category = assetCategoryBuffer;
    std::string path = currentTexturePath;
    AssetType type = assetTypes[selectedAssetType].type;

    if (type == AssetType::TILESET) {
        assetManager.LoadAsset(assetId, assetName, category, path,
                              tilesetTileWidth, tilesetTileHeight);
        UI::SetDebugMessage("[ASSET IMPORTER] Created tileset: " + assetName);

    } else if (type == AssetType::ANIMATED_SPRITESHEET ||
               type == AssetType::PLAYER ||
               type == AssetType::ENEMY) {

        if (frameSelected && frameWidth > 0 && frameHeight > 0) {
            assetManager.LoadAssetWithFrame(
                assetId, assetName, category, path,
                frameWidth, frameHeight,
                selectedFrameX, selectedFrameY,
                type
            );
            UI::SetDebugMessage("[ASSET IMPORTER] Created animated asset with frame (" +
                              std::to_string(selectedFrameX) + ", " +
                              std::to_string(selectedFrameY) + "): " + assetName);
        } else {
            assetManager.LoadAsset(assetId, assetName, category, path);
            UI::SetDebugMessage("[ASSET IMPORTER] Created animated asset (full texture): " + assetName);
        }

    } else if (type == AssetType::STATIC_SPRITESHEET) {
        if (frameSelected && frameWidth > 0 && frameHeight > 0) {
            assetManager.LoadAssetWithFrame(
                assetId, assetName, category, path,
                frameWidth, frameHeight,
                selectedFrameX, selectedFrameY,
                type
            );
            UI::SetDebugMessage("[ASSET IMPORTER] Created static sprite with selected frame: " + assetName);
        } else {
            assetManager.LoadAsset(assetId, assetName, category, path);
            UI::SetDebugMessage("[ASSET IMPORTER] Created static sprite (full texture): " + assetName);
        }

    } else {
        assetManager.LoadAsset(assetId, assetName, category, path);
        UI::SetDebugMessage("[ASSET IMPORTER] Created texture asset: " + assetName);
    }

    ImGui::OpenPopup("Asset Created");
    ResetForm();
}

void AssetImporter::ResetForm() {
    assetNameBuffer[0] = '\0';
    assetIdBuffer[0] = '\0';
    strcpy(assetCategoryBuffer, "Uncategorized");
    selectedAssetType = 0;
    selectedFrameX = 0;
    selectedFrameY = 0;
    frameSelected = false;
}

void AssetImporter::ScanForPngFiles(const std::string& directory) {
    foundPngFiles.clear();
    selectedFileIndex = -1;

    try {
        if (!fs::exists(directory)) {
            UI::SetDebugMessage("[ASSET IMPORTER] Directory not found: " + directory);
            return;
        }

        fs::path basePath = fs::path(directory);

        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".png") {
                    FileEntry fileEntry;
                    fileEntry.fullPath = entry.path().string();
                    fileEntry.filename = entry.path().filename().string();
                    fs::path relativePath = fs::relative(entry.path(), basePath);
                    fileEntry.displayPath = relativePath.string();
                    foundPngFiles.push_back(fileEntry);
                }
            }
        }

        std::sort(foundPngFiles.begin(), foundPngFiles.end(),
            [](const FileEntry& a, const FileEntry& b) {
                return a.displayPath < b.displayPath;
            });

        UI::SetDebugMessage("[ASSET IMPORTER] Found " +
                           std::to_string(foundPngFiles.size()) + " PNG files");
    } catch (const std::exception& e) {
        UI::SetDebugMessage("[ASSET IMPORTER] Error scanning: " + std::string(e.what()));
    }
}

void AssetImporter::LoadTexture(const std::string& path) {
    UnloadCurrentTexture();

    if (FileExists(path.c_str())) {
        currentTexture = ::LoadTexture(path.c_str());
        if (currentTexture.id != 0) {
            SetTextureFilter(currentTexture, TEXTURE_FILTER_POINT);
            textureLoaded = true;
            currentTexturePath = path;

            // SMART DEFAULT: If texture is larger than 64x64, assume it's a spritesheet
            // and set a reasonable frame size
            if (currentTexture.width > 64 || currentTexture.height > 64) {
                // Try common frame sizes
                if (currentTexture.width % 32 == 0 && currentTexture.height % 32 == 0) {
                    frameWidth = 32;
                    frameHeight = 32;
                } else if (currentTexture.width % 48 == 0 && currentTexture.height % 48 == 0) {
                    frameWidth = 48;
                    frameHeight = 48;
                } else if (currentTexture.width % 16 == 0 && currentTexture.height % 16 == 0) {
                    frameWidth = 16;
                    frameHeight = 16;
                } else if (currentTexture.width % 64 == 0 && currentTexture.height % 64 == 0) {
                    frameWidth = 64;
                    frameHeight = 64;
                } else {
                    // Can't guess - default to 32x32
                    frameWidth = 32;
                    frameHeight = 32;
                }
            } else {
                // Small texture - use full size
                frameWidth = currentTexture.width;
                frameHeight = currentTexture.height;
            }

            RecalculateGridSize();

            selectedFrameX = 0;
            selectedFrameY = 0;
            frameSelected = true;

            UI::SetDebugMessage("[ASSET IMPORTER] Loaded: " + path +
                              " (Frame size: " + std::to_string(frameWidth) + "x" +
                              std::to_string(frameHeight) + ")");
        } else {
            UI::SetDebugMessage("[ASSET IMPORTER] Failed to load: " + path);
        }
    } else {
        UI::SetDebugMessage("[ASSET IMPORTER] File not found: " + path);
    }
}

void AssetImporter::UnloadCurrentTexture() {
    if (textureLoaded) {
        UnloadTexture(currentTexture);
        currentTexture = {0};
        textureLoaded = false;
        currentTexturePath.clear();
    }
}

void AssetImporter::RecalculateGridSize() {
    if (textureLoaded && frameWidth > 0 && frameHeight > 0) {
        columnsInSheet = currentTexture.width / frameWidth;
        rowsInSheet = currentTexture.height / frameHeight;

        if (selectedFrameX >= columnsInSheet) {
            selectedFrameX = std::max(0, columnsInSheet - 1);
        }
        if (selectedFrameY >= rowsInSheet) {
            selectedFrameY = std::max(0, rowsInSheet - 1);
        }
    } else {
        columnsInSheet = 0;
        rowsInSheet = 0;
    }
}

std::string AssetImporter::GenerateUniqueId(const std::string& baseName, AssetManager& assetManager) {
    std::string id = baseName;

    int counter = 1;
    while (assetManager.GetAsset(id) != nullptr) {
        id = baseName + "_" + std::to_string(counter);
        counter++;
    }

    return id;
}
