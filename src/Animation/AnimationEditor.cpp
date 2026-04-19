#include "AnimationEditor.h"
#include "Engine.h"
#include "imgui.h"
#include "UI/UI.h"
#include <filesystem>
#include <algorithm>
#include <cstring>
#include "../Managers/LocalizationManager.h"

#define TR(key) Localization::Get(key)

namespace fs = std::filesystem;

AnimationEditor::~AnimationEditor() {
    UnloadCurrentTexture();
}

void AnimationEditor::Render(Engine& engine) {
    if (!windowOpen) return;
    activeEngine = &engine;

    ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(TR("animation_editor.window_title"), &windowOpen)) {

        // Left panel - file selection and settings
        ImGui::BeginChild("LeftPanel", ImVec2(320, 0), true);

        RenderFileSelector();
        ImGui::Separator();
        RenderFrameSettings();
        ImGui::Separator();
        RenderAnimationDefiner();
        ImGui::Separator();
        RenderDefinedAnimationsList();
        ImGui::Separator();
        RenderSaveSection(engine);

        ImGui::EndChild();

        ImGui::SameLine();

        // Right panel - sprite sheet preview
        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);

        RenderSpriteSheetPreview();
        ImGui::Separator();
        RenderAnimationPreview(engine);

        ImGui::EndChild();
    }
    ImGui::End();

    // Update preview animation
    UpdatePreview(GetFrameTime());
}

void AnimationEditor::RenderFileSelector() {
    ImGui::Text("%s", TR("animation_editor.sprite_sheet_selection"));
    ImGui::Spacing();

    // Search path input
    ImGui::Text("%s", TR("animation_editor.search_path"));
    if (ImGui::InputText("##SearchPath", searchPathBuffer, sizeof(searchPathBuffer))) {
        needsRescan = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(TR("animation_editor.scan"))) {
        ScanForPngFiles(searchPathBuffer);
    }

    // File list
    ImGui::Text(TR("animation_editor.found_png_files"), (int)foundPngFiles.size());

    ImGui::BeginChild("FileList", ImVec2(0, 150), true);
    for (int i = 0; i < (int)foundPngFiles.size(); i++) {
        ImGui::PushID(i);  // Use index as unique ID to avoid conflicts

        if (ImGui::Selectable(foundPngFiles[i].displayPath.c_str(), selectedFileIndex == i)) {
            selectedFileIndex = i;
            LoadTexture(foundPngFiles[i].fullPath);
        }

        // Tooltip with full path
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", foundPngFiles[i].fullPath.c_str());
        }

        ImGui::PopID();
    }
    ImGui::EndChild();

    // Show currently loaded file
    if (textureLoaded) {
        ImGui::TextWrapped(TR("animation_editor.loaded_path"), currentTexturePath.c_str());
        ImGui::Text(TR("animation_editor.size"), currentTexture.width, currentTexture.height);
    }
}

void AnimationEditor::RenderFrameSettings() {
    ImGui::Text("%s", TR("animation_editor.frame_settings"));
    ImGui::Spacing();

    bool changed = false;

    ImGui::Text("%s", TR("animation_editor.frame_size"));
    ImGui::SetNextItemWidth(100);
    std::string frameWidthLabel = std::string(TR("animation_editor.width")) + "##Frame";
    if (ImGui::InputInt(frameWidthLabel.c_str(), &frameWidth, 8, 16)) {
        if (frameWidth < 8) frameWidth = 8;
        if (frameWidth > 256) frameWidth = 256;
        changed = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    std::string frameHeightLabel = std::string(TR("animation_editor.height")) + "##Frame";
    if (ImGui::InputInt(frameHeightLabel.c_str(), &frameHeight, 8, 16)) {
        if (frameHeight < 8) frameHeight = 8;
        if (frameHeight > 256) frameHeight = 256;
        changed = true;
    }

    // Quick presets
    ImGui::Text("%s", TR("animation_editor.presets"));
    if (ImGui::Button("16x16")) { frameWidth = 16; frameHeight = 16; changed = true; }
    ImGui::SameLine();
    if (ImGui::Button("32x32")) { frameWidth = 32; frameHeight = 32; changed = true; }
    ImGui::SameLine();
    if (ImGui::Button("32x48")) { frameWidth = 32; frameHeight = 48; changed = true; }
    if (ImGui::Button("48x48")) { frameWidth = 48; frameHeight = 48; changed = true; }
    ImGui::SameLine();
    if (ImGui::Button("64x64")) { frameWidth = 64; frameHeight = 64; changed = true; }

    if (changed) {
        RecalculateGridSize();
    }

    Texture2D sourceTexture = ResolveTextureForAssetId(sourceAssetIdBuffer);
    if (IsTextureUsable(sourceTexture)) {
        const int cols = std::max(1, sourceTexture.width / frameWidth);
        const int rows = std::max(1, sourceTexture.height / frameHeight);
        ImGui::Text(TR("animation_editor.grid"), cols, rows);
    } else if (textureLoaded) {
        ImGui::Text(TR("animation_editor.grid"), columnsInSheet, rowsInSheet);
    }
}

void AnimationEditor::RenderSpriteSheetPreview() {
    ImGui::Text("%s", TR("animation_editor.sprite_sheet_preview"));
    ImGui::Spacing();

    std::string previewLabel;
    bool sourceMissing = false;
    Texture2D previewTexture = ResolveTextureForAssetId(sourceAssetIdBuffer, &previewLabel, &sourceMissing);
    if (!IsTextureUsable(previewTexture)) {
        previewTexture = currentTexture;
        previewLabel = currentTexturePath.empty() ? "<none>" : currentTexturePath;
    }

    if (!IsTextureUsable(previewTexture)) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", TR("animation_editor.no_sprite_sheet_loaded"));
        return;
    }
    ImGui::Text(TR("animation_editor.sheet_source"), previewLabel.c_str());
    if (sourceMissing) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "%s", TR("animation_editor.source_asset_missing_showing_fallback_sheet"));
    }

    const int localColumns = std::max(1, previewTexture.width / frameWidth);
    const int localRows = std::max(1, previewTexture.height / frameHeight);
    columnsInSheet = localColumns;
    rowsInSheet = localRows;

    // Calculate display size (fit to available space)
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    availableSize.y -= 200; // Reserve space for animation preview below

    float scaleX = availableSize.x / previewTexture.width;
    float scaleY = availableSize.y / previewTexture.height;
    float scale = std::min(scaleX, scaleY);
    scale = std::max(scale, 0.5f); // Minimum scale
    scale = std::min(scale, 4.0f); // Maximum scale

    float displayWidth = previewTexture.width * scale;
    float displayHeight = previewTexture.height * scale;

    // Get cursor position for drawing grid overlay
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // Draw the texture
    ImGui::Image((ImTextureID)(intptr_t)previewTexture.id,
                 ImVec2(displayWidth, displayHeight));

    // Draw grid overlay
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float scaledFrameW = frameWidth * scale;
    float scaledFrameH = frameHeight * scale;

    // Vertical lines
    for (int col = 0; col <= localColumns; col++) {
        float x = cursorPos.x + col * scaledFrameW;
        drawList->AddLine(
            ImVec2(x, cursorPos.y),
            ImVec2(x, cursorPos.y + displayHeight),
            IM_COL32(255, 255, 0, 128), 1.0f
        );
    }

    // Horizontal lines
    for (int row = 0; row <= localRows; row++) {
        float y = cursorPos.y + row * scaledFrameH;
        drawList->AddLine(
            ImVec2(cursorPos.x, y),
            ImVec2(cursorPos.x + displayWidth, y),
            IM_COL32(255, 255, 0, 128), 1.0f
        );
    }

    // Highlight selected row
    if (selectedRow >= 0 && selectedRow < localRows) {
        float y = cursorPos.y + selectedRow * scaledFrameH;
        drawList->AddRectFilled(
            ImVec2(cursorPos.x, y),
            ImVec2(cursorPos.x + displayWidth, y + scaledFrameH),
            IM_COL32(0, 255, 0, 50)
        );
    }

    // Row labels
    for (int row = 0; row < localRows; row++) {
        float y = cursorPos.y + row * scaledFrameH + scaledFrameH / 2 - 6;
        char label[16];
        snprintf(label, sizeof(label), TR("animation_editor.row_label"), row);
        drawList->AddText(ImVec2(cursorPos.x + 4, y), IM_COL32(255, 255, 255, 255), label);
    }

}

void AnimationEditor::RenderAnimationDefiner() {
    ImGui::Text("%s", TR("animation_editor.define_animation"));
    ImGui::Spacing();

    // Animation base name (without direction suffix)
    ImGui::Text("%s", TR("animation_editor.base_name"));
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##AnimName", nameBuffer, sizeof(nameBuffer));

    ImGui::Text("%s", TR("animation_editor.source_asset_id"));
    ImGui::SetNextItemWidth(-1);
    bool sourceIdChanged = ImGui::InputText("##AnimSourceAssetID", sourceAssetIdBuffer, sizeof(sourceAssetIdBuffer));
    if (targetAsset) {
        if (ImGui::Button(TR("animation_editor.use_target_asset"), ImVec2(-1, 0))) {
            strncpy(sourceAssetIdBuffer, targetAsset->id.c_str(), sizeof(sourceAssetIdBuffer) - 1);
            sourceAssetIdBuffer[sizeof(sourceAssetIdBuffer) - 1] = '\0';
            sourceIdChanged = true;
        }
    }

    std::string resolvedLabel;
    bool sourceMissing = false;
    Texture2D sourceTexture = ResolveTextureForAssetId(sourceAssetIdBuffer, &resolvedLabel, &sourceMissing);
    const bool hasSourceTexture = IsTextureUsable(sourceTexture);
    const int sourceColumns = hasSourceTexture ? std::max(1, sourceTexture.width / frameWidth) : 1;
    const int sourceRows = hasSourceTexture ? std::max(1, sourceTexture.height / frameHeight) : 1;

    if (selectedRow >= sourceRows) selectedRow = sourceRows - 1;
    if (selectedFrameCount > sourceColumns) selectedFrameCount = sourceColumns;
    if (selectedFrameCount < 1) selectedFrameCount = 1;

    if (sourceIdChanged) {
        columnsInSheet = sourceColumns;
        rowsInSheet = sourceRows;
    }

    if (!hasSourceTexture) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "%s", TR("animation_editor.no_loaded_texture_for_source_asset_id"));
    } else {
        ImGui::Text(TR("animation_editor.source_sheet"), resolvedLabel.c_str(), sourceTexture.width, sourceTexture.height);
    }
    if (sourceMissing) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "%s", TR("animation_editor.source_asset_missing_using_fallback"));
    }

    // Row selection
    ImGui::Text("%s", TR("animation_editor.row"));
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderInt("##Row", &selectedRow, 0, std::max(0, sourceRows - 1))) {
        // Row changed
    }

    // Frame count
    ImGui::Text("%s", TR("animation_editor.frame_count"));
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##FrameCount", &selectedFrameCount, 1, sourceColumns);

    // Frame rate
    ImGui::Text("%s", TR("animation_editor.frame_rate_fps"));
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##FrameRate", &selectedFrameRate, 1.0f, 30.0f, "%.1f");

    // Loop checkbox
    ImGui::Checkbox(TR("animation_editor.loop"), &selectedLoop);

    // Trigger type
    ImGui::Text("%s", TR("animation_editor.trigger"));
    const char* triggerOptions[] = {
        TR("animation_editor.trigger.loop"),
        TR("animation_editor.trigger.input"),
        TR("animation_editor.trigger.event"),
        TR("animation_editor.trigger.idle")
    };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##Trigger", &selectedTrigger, triggerOptions, 4);

    // Direction
    ImGui::Text("%s", TR("animation_editor.direction"));
    const char* directionOptions[] = {
        TR("animation_editor.direction.none"),
        TR("animation_editor.direction.down"),
        TR("animation_editor.direction.up"),
        TR("animation_editor.direction.right"),
        TR("animation_editor.direction.left")
    };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##Direction", &selectedDirection, directionOptions, 5);

    // Auto-create opposite horizontal direction checkbox (LEFT <-> RIGHT)
    if (selectedDirection == static_cast<int>(AnimationDirection::RIGHT) ||
        selectedDirection == static_cast<int>(AnimationDirection::LEFT)) {
        ImGui::Checkbox(TR("animation_editor.auto_create_opposite_flipped"), &autoCreateOppositeHorizontal);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", TR("animation_editor.auto_create_opposite_tooltip"));
        }
    }

    ImGui::Spacing();

    // Build the full animation name for preview
    AnimationDirection dir = static_cast<AnimationDirection>(selectedDirection);
    std::string fullName = std::string(nameBuffer) + GetDirectionSuffix(dir);
    ImGui::Text(TR("animation_editor.will_create"), fullName.c_str());
    if ((selectedDirection == static_cast<int>(AnimationDirection::RIGHT) ||
         selectedDirection == static_cast<int>(AnimationDirection::LEFT)) &&
        autoCreateOppositeHorizontal) {
        const AnimationDirection oppositeDir =
            (dir == AnimationDirection::RIGHT) ? AnimationDirection::LEFT : AnimationDirection::RIGHT;
        std::string oppositeName = std::string(nameBuffer) + GetDirectionSuffix(oppositeDir);
        ImGui::Text(TR("animation_editor.will_create_flipped"), oppositeName.c_str());
    }

    ImGui::Spacing();

    // Add button
    if (ImGui::Button(TR("animation_editor.add_animation"), ImVec2(-1, 0))) {
        std::string baseName = nameBuffer;

        if (baseName.empty()) {
            UI::SetDebugMessage("[ANIMATION EDITOR] Animation name cannot be empty");
        } else {
            AnimationDefinition def;
            def.name = fullName;
            def.sourceAssetId = sourceAssetIdBuffer;
            def.row = selectedRow;
            def.frameCount = selectedFrameCount;
            def.frameRate = selectedFrameRate;
            def.loop = selectedLoop;
            def.trigger = static_cast<AnimationTrigger>(selectedTrigger);
            def.direction = dir;
            def.isAutoFlipped = false;

            // Check for duplicate names
            bool duplicate = false;
            for (const auto& existing : definedAnimations) {
                if (existing.name == def.name) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate) {
                UI::SetDebugMessage("[ANIMATION EDITOR] Animation '" + def.name + "' already exists");
            } else {
                definedAnimations.push_back(def);
                UI::SetDebugMessage("[ANIMATION EDITOR] Added animation: " + def.name);

                // Auto-create opposite horizontal variant if requested.
                if ((dir == AnimationDirection::RIGHT || dir == AnimationDirection::LEFT) &&
                    autoCreateOppositeHorizontal) {
                    const AnimationDirection oppositeDir =
                        (dir == AnimationDirection::RIGHT) ? AnimationDirection::LEFT : AnimationDirection::RIGHT;
                    AnimationDefinition oppositeDef = def;
                    oppositeDef.name = baseName + GetDirectionSuffix(oppositeDir);
                    oppositeDef.direction = oppositeDir;
                    oppositeDef.isAutoFlipped = true;

                    // Check for duplicate
                    bool oppositeDuplicate = false;
                    for (const auto& existing : definedAnimations) {
                        if (existing.name == oppositeDef.name) {
                            oppositeDuplicate = true;
                            break;
                        }
                    }

                    if (!oppositeDuplicate) {
                        definedAnimations.push_back(oppositeDef);
                        UI::SetDebugMessage("[ANIMATION EDITOR] Auto-created: " + oppositeDef.name + " (flipped)");
                    }
                }
            }
        }
    }
}


void AnimationEditor::RenderDefinedAnimationsList() {
    ImGui::Text(TR("animation_editor.defined_animations"), (int)definedAnimations.size());
    ImGui::Spacing();

    if (definedAnimations.empty()) {
        ImGui::TextDisabled("%s", TR("animation_editor.no_animations_defined"));
        return;
    }

    ImGui::BeginChild("AnimList", ImVec2(0, 150), true);

    int toDelete = -1;

    for (int i = 0; i < (int)definedAnimations.size(); i++) {
        ImGui::PushID(i);

        const auto& anim = definedAnimations[i];
        bool isSelected = (i == previewAnimIndex);

        // FIX: Store the cursor position and handle delete button FIRST
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

        // Delete button on the left (before selectable)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));

        if (ImGui::SmallButton("X")) {
            toDelete = i;
        }

        ImGui::PopStyleColor(3);

        bool buttonHovered = ImGui::IsItemHovered();

        ImGui::SameLine();

        // Now the selectable - only process if button not hovered
        ImGui::SetNextItemWidth(-30);  // Reserve space for the button we already drew
        bool clicked = ImGui::Selectable("##select", isSelected, 0, ImVec2(0, 20));

        // Only change preview if clicking selectable AND not hovering button
        if (clicked && !buttonHovered) {
            previewAnimIndex = i;
            previewFrame = 0;
            previewTimer = 0.0f;
            previewFlipped = anim.isAutoFlipped;
            const std::string selectedSourceId = anim.sourceAssetId.empty() && targetAsset ? targetAsset->id : anim.sourceAssetId;
            strncpy(sourceAssetIdBuffer, selectedSourceId.c_str(), sizeof(sourceAssetIdBuffer) - 1);
            sourceAssetIdBuffer[sizeof(sourceAssetIdBuffer) - 1] = '\0';
        }

        ImGui::SameLine(30);  // Move back to where the selectable started

        // Show different color for auto-flipped animations
        if (anim.isAutoFlipped) {
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
                TR("animation_editor.row_frames_flipped"),
                anim.name.c_str(), anim.row, anim.frameCount);
        } else {
            ImGui::Text(TR("animation_editor.row_frames"),
                anim.name.c_str(), anim.row, anim.frameCount);
        }

        ImGui::PopStyleVar();
        ImGui::PopID();
    }

    ImGui::EndChild();

    // Delete if requested
    if (toDelete >= 0) {
        definedAnimations.erase(definedAnimations.begin() + toDelete);
        if (previewAnimIndex >= (int)definedAnimations.size()) {
            previewAnimIndex = -1;
        }
    }

    // Clear all button
    if (!definedAnimations.empty()) {
        if (ImGui::Button(TR("animation_editor.clear_all"))) {
            definedAnimations.clear();
            previewAnimIndex = -1;
        }
    }
}

void AnimationEditor::RenderAnimationPreview(Engine& engine) {
    ImGui::Text("%s", TR("animation_editor.animation_preview"));
    ImGui::Spacing();
    (void)engine;

    if (previewAnimIndex < 0 || previewAnimIndex >= (int)definedAnimations.size()) {
        ImGui::TextDisabled("%s", TR("animation_editor.select_animation_to_preview"));
        return;
    }

    const auto& anim = definedAnimations[previewAnimIndex];
    Texture2D previewTexture = currentTexture;
    std::string previewSource = currentTexturePath.empty() ? "<current texture>" : currentTexturePath;
    bool usingFallbackTexture = false;

    const std::string sourceId = anim.sourceAssetId.empty() && targetAsset ? targetAsset->id : anim.sourceAssetId;
    if (!sourceId.empty()) {
        bool missing = false;
        Texture2D resolved = ResolveTextureForAssetId(sourceId, &previewSource, &missing);
        if (IsTextureUsable(resolved)) {
            previewTexture = resolved;
        }
        usingFallbackTexture = missing;
    }

    if (previewTexture.id == 0 || previewTexture.width <= 0 || previewTexture.height <= 0) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "%s", TR("animation_editor.no_preview_texture_resolved"));
        return;
    }

    ImGui::Text(TR("animation_editor.playing"),
                anim.name.c_str(), previewFrame + 1, anim.frameCount,
                anim.isAutoFlipped ? TR("animation_editor.flipped_suffix") : "");
    ImGui::Text(TR("animation_editor.source"), previewSource.c_str());
    if (usingFallbackTexture) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "%s", TR("animation_editor.source_asset_missing_showing_fallback_texture"));
    }

    // Calculate source rectangle for current frame
    float srcX = static_cast<float>(previewFrame * frameWidth);
    float srcY = static_cast<float>(anim.row * frameHeight);
    float srcW = static_cast<float>(frameWidth);
    float srcH = static_cast<float>(frameHeight);

    // Display at 3x scale for visibility
    float scale = 3.0f;
    ImVec2 displaySize(frameWidth * scale, frameHeight * scale);

    // UV coordinates for the frame
    ImVec2 uv0, uv1;

    if (anim.isAutoFlipped) {
        // Flip horizontally by swapping U coordinates
        uv0 = ImVec2((srcX + srcW) / previewTexture.width,
                     srcY / previewTexture.height);
        uv1 = ImVec2(srcX / previewTexture.width,
                     (srcY + srcH) / previewTexture.height);
    } else {
        uv0 = ImVec2(srcX / previewTexture.width,
                     srcY / previewTexture.height);
        uv1 = ImVec2((srcX + srcW) / previewTexture.width,
                     (srcY + srcH) / previewTexture.height);
    }

    ImGui::Image((ImTextureID)(intptr_t)previewTexture.id, displaySize, uv0, uv1);
}

void AnimationEditor::RenderSaveSection(Engine& engine) {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", TR("animation_editor.save_animations"));

    if (targetAsset) {
        ImGui::Text(TR("animation_editor.target_asset"), targetAsset->name.c_str());
        ImGui::Text(TR("animation_editor.animations_count"), definedAnimations.size());

        if (definedAnimations.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%s", TR("animation_editor.no_animations_defined"));
        }

        ImGui::Separator();

        if (ImGui::Button(TR("animation_editor.save_to_asset"), ImVec2(-1, 0))) {
            SaveAnimationsToAsset(engine);
        }

        if (ImGui::Button(TR("animation_editor.close_editor"), ImVec2(-1, 0))) {
            windowOpen = false;
            targetAsset = nullptr;
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                          "%s", TR("animation_editor.no_target_asset"));
    }
}

void AnimationEditor::LoadTexture(const std::string& path) {
    UnloadCurrentTexture();

    if (FileExists(path.c_str())) {
        currentTexture = ::LoadTexture(path.c_str());
        if (currentTexture.id != 0) {
            // Set pixel-perfect filtering
            SetTextureFilter(currentTexture, TEXTURE_FILTER_POINT);

            textureLoaded = true;
            currentTexturePath = path;
            RecalculateGridSize();

            // Clear previous definitions when loading new texture
            definedAnimations.clear();
            previewAnimIndex = -1;

            UI::SetDebugMessage("[ANIMATION EDITOR] Loaded: " + path);
        } else {
            UI::SetDebugMessage("[ANIMATION EDITOR] Failed to load: " + path);
        }
    } else {
        UI::SetDebugMessage("[ANIMATION EDITOR] File not found: " + path);
    }
}

void AnimationEditor::UnloadCurrentTexture() {
    if (textureLoaded) {
        ::UnloadTexture(currentTexture);
        currentTexture = {0};
        textureLoaded = false;
        currentTexturePath.clear();
    }
}

void AnimationEditor::ScanForPngFiles(const std::string& directory) {
    foundPngFiles.clear();
    selectedFileIndex = -1;
    currentSearchPath = directory;

    try {
        if (!fs::exists(directory)) {
            UI::SetDebugMessage("[ANIMATION EDITOR] Directory not found: " + directory);
            return;
        }

        fs::path basePath = fs::path(directory);

        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // Convert to lowercase for comparison
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".png") {
                    FileEntry fileEntry;
                    fileEntry.fullPath = entry.path().string();

                    // Create relative path for display
                    fs::path relativePath = fs::relative(entry.path(), basePath);
                    fileEntry.displayPath = relativePath.string();

                    foundPngFiles.push_back(fileEntry);
                }
            }
        }

        // Sort alphabetically by display path
        std::sort(foundPngFiles.begin(), foundPngFiles.end(),
            [](const FileEntry& a, const FileEntry& b) {
                return a.displayPath < b.displayPath;
            });

        UI::SetDebugMessage("[ANIMATION EDITOR] Found " +
                           std::to_string(foundPngFiles.size()) + " PNG files");
    } catch (const std::exception& e) {
        UI::SetDebugMessage("[ANIMATION EDITOR] Error scanning: " + std::string(e.what()));
    }
}

void AnimationEditor::UpdatePreview(float deltaTime) {
    if (previewAnimIndex < 0 || previewAnimIndex >= (int)definedAnimations.size()) {
        return;
    }

    const auto& anim = definedAnimations[previewAnimIndex];
    float frameDuration = 1.0f / anim.frameRate;

    previewTimer += deltaTime;

    while (previewTimer >= frameDuration) {
        previewTimer -= frameDuration;
        previewFrame++;

        if (previewFrame >= anim.frameCount) {
            if (anim.loop) {
                previewFrame = 0;
            } else {
                previewFrame = anim.frameCount - 1;
            }
        }
    }
}

void AnimationEditor::RecalculateGridSize() {
    if (textureLoaded && frameWidth > 0 && frameHeight > 0) {
        RecalculateGridSizeFromTexture(currentTexture);
    } else {
        columnsInSheet = 0;
        rowsInSheet = 0;
    }
}

void AnimationEditor::RecalculateGridSizeFromTexture(const Texture2D& texture) {
    if (!IsTextureUsable(texture) || frameWidth <= 0 || frameHeight <= 0) {
        columnsInSheet = 0;
        rowsInSheet = 0;
        return;
    }

    columnsInSheet = std::max(1, texture.width / frameWidth);
    rowsInSheet = std::max(1, texture.height / frameHeight);

    if (selectedRow >= rowsInSheet) {
        selectedRow = std::max(0, rowsInSheet - 1);
    }
    if (selectedFrameCount > columnsInSheet) {
        selectedFrameCount = columnsInSheet;
    }
}

Texture2D AnimationEditor::ResolveTextureForAssetId(const std::string& assetId, std::string* resolvedLabel, bool* missing) const {
    if (missing) *missing = false;
    if (resolvedLabel) *resolvedLabel = "";

    if (activeEngine) {
        std::string id = assetId;
        if (id.empty() && targetAsset) {
            id = targetAsset->id;
        }
        if (!id.empty()) {
            Asset* sourceAsset = activeEngine->GetAssetManager().GetAsset(id);
            if (sourceAsset && sourceAsset->loaded && sourceAsset->texture.id != 0) {
                if (resolvedLabel) *resolvedLabel = sourceAsset->id;
                return sourceAsset->texture;
            }
            if (missing) *missing = true;
            if (resolvedLabel) *resolvedLabel = id + " (missing)";
        }
    }

    if (textureLoaded && currentTexture.id != 0) {
        if (resolvedLabel) *resolvedLabel = currentTexturePath.empty() ? "<loaded texture>" : currentTexturePath;
        return currentTexture;
    }

    return Texture2D{0};
}

bool AnimationEditor::IsTextureUsable(const Texture2D& texture) {
    return texture.id != 0 && texture.width > 0 && texture.height > 0;
}

std::string AnimationEditor::GetDirectionSuffix(AnimationDirection dir) const {
    switch (dir) {
        case AnimationDirection::DOWN:  return "_down";
        case AnimationDirection::UP:    return "_up";
        case AnimationDirection::RIGHT: return "_right";
        case AnimationDirection::LEFT:  return "_left";
        case AnimationDirection::NONE:
        default: return "";
    }
}

void AnimationEditor::OpenForAsset(Asset* asset) {
    if (!asset) return;

    targetAsset = asset;
    windowOpen = true;
    if (targetAsset) {
        strncpy(sourceAssetIdBuffer, targetAsset->id.c_str(), sizeof(sourceAssetIdBuffer) - 1);
        sourceAssetIdBuffer[sizeof(sourceAssetIdBuffer) - 1] = '\0';
    } else {
        sourceAssetIdBuffer[0] = '\0';
    }

    // Load the asset's texture
    if (!asset->path.empty()) {
        LoadTexture(asset->path);
    }

    // Clear any existing animation definitions
    definedAnimations.clear();

    // Pre-load existing animations if the asset has any
    if (asset->HasAnimations() && asset->animationSet) {
        const auto& animSet = asset->animationSet;

        // Set frame dimensions from animation set
        if (animSet->frameWidth > 0 && animSet->frameHeight > 0) {
            frameWidth = animSet->frameWidth;
            frameHeight = animSet->frameHeight;
            RecalculateGridSize();
        }

        // Convert each Animation to AnimationDefinition for editing
        for (const auto& [name, anim] : animSet->animations) {
            AnimationDefinition def;
            def.name = name;
            def.sourceAssetId = anim.sourceAssetId.empty() ? targetAsset->id : anim.sourceAssetId;
            def.loop = anim.loop;
            def.isAutoFlipped = anim.flipHorizontallyAtRuntime;
            def.trigger = anim.trigger;
            def.direction = anim.direction;

            // Reverse-engineer row and frame count from first frame
            if (!anim.frames.empty()) {
                const auto& firstFrame = anim.frames[0];
                def.row = static_cast<int>(firstFrame.sourceRect.y / frameHeight);
                def.frameCount = static_cast<int>(anim.frames.size());

                // Calculate frame rate from frame duration
                if (firstFrame.duration > 0.0f) {
                    def.frameRate = 1.0f / firstFrame.duration;
                }
            }

            definedAnimations.push_back(def);
        }
    }

    UI::SetDebugMessage("[ANIMATION] Opened editor for asset: " + asset->name +
                       " (" + std::to_string(definedAnimations.size()) + " animations loaded)");
}

void AnimationEditor::SaveAnimationsToAsset(Engine& engine) {
    if (!targetAsset) {
        UI::SetDebugMessage("[ERROR] No target asset set!");
        return;
    }

    if (definedAnimations.empty()) {
        UI::SetDebugMessage("[WARNING] No animations to save!");
        return;
    }

    // Create or update the AnimationSet
    if (!targetAsset->animationSet) {
        targetAsset->animationSet = std::make_unique<AnimationSet>();
    }

    AnimationSet* animSet = targetAsset->animationSet.get();

    // Clear existing animations
    animSet->animations.clear();

    // Set basic info
    animSet->id = targetAsset->id + "_animations";
    animSet->textureId = targetAsset->id;
    // Keep animation texture bound to the asset texture used at runtime.
    animSet->texture = targetAsset->texture;
    animSet->textureLoaded = targetAsset->loaded;
    animSet->frameWidth = frameWidth;
    animSet->frameHeight = frameHeight;

    AssetManager& assetManager = engine.GetAssetManager();

    // Convert each AnimationDefinition to Animation
    for (const auto& def : definedAnimations) {
        animSet->AddAnimationFromRow(
                def.name,
                def.row,
                def.frameCount,
                def.frameRate,
                def.loop,
                def.isAutoFlipped,
                def.trigger,
                def.direction,
                def.sourceAssetId.empty() ? targetAsset->id : def.sourceAssetId);

        Animation* savedAnim = animSet->GetAnimation(def.name);
        if (!savedAnim) continue;

        const std::string sourceId = savedAnim->sourceAssetId.empty() ? targetAsset->id : savedAnim->sourceAssetId;
        Asset* sourceAsset = assetManager.GetAsset(sourceId);
        if (sourceAsset && sourceAsset->loaded) {
            savedAnim->sourceTexture = sourceAsset->texture;
            savedAnim->hasSourceTexture = true;
        } else {
            savedAnim->sourceTexture = targetAsset->texture;
            savedAnim->hasSourceTexture = true;
            if (sourceId != targetAsset->id) {
                UI::SetDebugMessage("[ANIMATION] Source asset not found for '" + def.name + "': " + sourceId + ". Using target texture.");
            }
        }
    }

    // Mark asset as having animations
    targetAsset->hasAnimations = true;

    UI::SetDebugMessage("[ANIMATION] Saved " + std::to_string(definedAnimations.size()) +
                       " animations to asset: " + targetAsset->name);
}
