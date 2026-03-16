#include "AnimationEditor.h"
#include "Engine.h"
#include "imgui.h"
#include "UI/UI.h"
#include <filesystem>
#include <algorithm>
#include "../Managers/LocalizitionManager.h"

#define TR(key) Localization::Get(key)

namespace fs = std::filesystem;

AnimationEditor::~AnimationEditor() {
    UnloadCurrentTexture();
}

void AnimationEditor::Render(Engine& engine) {
    if (!windowOpen) return;

    ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Animation Editor", &windowOpen)) {

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
        RenderAnimationPreview();

        ImGui::EndChild();
    }
    ImGui::End();

    // Update preview animation
    UpdatePreview(GetFrameTime());
}

void AnimationEditor::RenderFileSelector() {
    ImGui::Text("Sprite Sheet Selection");
    ImGui::Spacing();

    // Search path input
    ImGui::Text("Search Path:");
    if (ImGui::InputText("##SearchPath", searchPathBuffer, sizeof(searchPathBuffer))) {
        needsRescan = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Scan")) {
        ScanForPngFiles(searchPathBuffer);
    }

    // File list
    ImGui::Text("PNG Files Found: %d", (int)foundPngFiles.size());

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
        ImGui::TextWrapped("Loaded: %s", currentTexturePath.c_str());
        ImGui::Text("Size: %dx%d", currentTexture.width, currentTexture.height);
    }
}

void AnimationEditor::RenderFrameSettings() {
    ImGui::Text("Frame Settings");
    ImGui::Spacing();

    bool changed = false;

    ImGui::Text("Frame Size:");
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputInt("Width##Frame", &frameWidth, 8, 16)) {
        if (frameWidth < 8) frameWidth = 8;
        if (frameWidth > 256) frameWidth = 256;
        changed = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputInt("Height##Frame", &frameHeight, 8, 16)) {
        if (frameHeight < 8) frameHeight = 8;
        if (frameHeight > 256) frameHeight = 256;
        changed = true;
    }

    // Quick presets
    ImGui::Text("Presets:");
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

    // Show grid info
    if (textureLoaded) {
        ImGui::Text("Grid: %d columns x %d rows", columnsInSheet, rowsInSheet);
    }
}

void AnimationEditor::RenderSpriteSheetPreview() {
    ImGui::Text("Sprite Sheet Preview");
    ImGui::Spacing();

    if (!textureLoaded) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "No sprite sheet loaded");
        return;
    }

    // Calculate display size (fit to available space)
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    availableSize.y -= 200; // Reserve space for animation preview below

    float scaleX = availableSize.x / currentTexture.width;
    float scaleY = availableSize.y / currentTexture.height;
    float scale = std::min(scaleX, scaleY);
    scale = std::max(scale, 0.5f); // Minimum scale
    scale = std::min(scale, 4.0f); // Maximum scale

    float displayWidth = currentTexture.width * scale;
    float displayHeight = currentTexture.height * scale;

    // Get cursor position for drawing grid overlay
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // Draw the texture
    ImGui::Image((ImTextureID)(intptr_t)currentTexture.id,
                 ImVec2(displayWidth, displayHeight));

    // Draw grid overlay
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float scaledFrameW = frameWidth * scale;
    float scaledFrameH = frameHeight * scale;

    // Vertical lines
    for (int col = 0; col <= columnsInSheet; col++) {
        float x = cursorPos.x + col * scaledFrameW;
        drawList->AddLine(
            ImVec2(x, cursorPos.y),
            ImVec2(x, cursorPos.y + displayHeight),
            IM_COL32(255, 255, 0, 128), 1.0f
        );
    }

    // Horizontal lines
    for (int row = 0; row <= rowsInSheet; row++) {
        float y = cursorPos.y + row * scaledFrameH;
        drawList->AddLine(
            ImVec2(cursorPos.x, y),
            ImVec2(cursorPos.x + displayWidth, y),
            IM_COL32(255, 255, 0, 128), 1.0f
        );
    }

    // Highlight selected row
    if (selectedRow >= 0 && selectedRow < rowsInSheet) {
        float y = cursorPos.y + selectedRow * scaledFrameH;
        drawList->AddRectFilled(
            ImVec2(cursorPos.x, y),
            ImVec2(cursorPos.x + displayWidth, y + scaledFrameH),
            IM_COL32(0, 255, 0, 50)
        );
    }

    // Row labels
    for (int row = 0; row < rowsInSheet; row++) {
        float y = cursorPos.y + row * scaledFrameH + scaledFrameH / 2 - 6;
        char label[16];
        snprintf(label, sizeof(label), "Row %d", row);
        drawList->AddText(ImVec2(cursorPos.x + 4, y), IM_COL32(255, 255, 255, 255), label);
    }

    ImVec2 uv_min = ImVec2(0.0f, 0.0f);
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);
    ImGui::Image((ImTextureID)&currentTexture.id, ImVec2(currentTexture.width, currentTexture.height));
}

void AnimationEditor::RenderAnimationDefiner() {
    ImGui::Text("Define Animation");
    ImGui::Spacing();

    if (!textureLoaded) {
        ImGui::TextDisabled("Load a sprite sheet first");
        return;
    }

    // Animation base name (without direction suffix)
    ImGui::Text("Base Name:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##AnimName", nameBuffer, sizeof(nameBuffer));

    // Row selection
    ImGui::Text("Row:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderInt("##Row", &selectedRow, 0, std::max(0, rowsInSheet - 1))) {
        // Row changed
    }

    // Frame count
    ImGui::Text("Frame Count:");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##FrameCount", &selectedFrameCount, 1, columnsInSheet);

    // Frame rate
    ImGui::Text("Frame Rate (FPS):");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##FrameRate", &selectedFrameRate, 1.0f, 30.0f, "%.1f");

    // Loop checkbox
    ImGui::Checkbox("Loop", &selectedLoop);

    // Trigger type
    ImGui::Text("Trigger:");
    const char* triggerOptions[] = { "LOOP", "INPUT", "EVENT", "IDLE" };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##Trigger", &selectedTrigger, triggerOptions, 4);

    // Direction (without LEFT option since it's auto-created)
    ImGui::Text("Direction:");
    const char* directionOptions[] = { "NONE", "DOWN", "UP", "RIGHT" };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##Direction", &selectedDirection, directionOptions, 4);

    // Auto-create LEFT checkbox (only show when RIGHT is selected)
    if (selectedDirection == 3) {  // RIGHT is index 3
        ImGui::Checkbox("Auto-create LEFT (flipped)", &autoCreateLeft);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Automatically creates a LEFT animation\nusing the RIGHT frames flipped horizontally");
        }
    }

    ImGui::Spacing();

    // Build the full animation name for preview
    AnimationDirection dir = static_cast<AnimationDirection>(selectedDirection);
    std::string fullName = std::string(nameBuffer) + GetDirectionSuffix(dir);
    ImGui::Text("Will create: %s", fullName.c_str());
    if (selectedDirection == 3 && autoCreateLeft) {
        std::string leftName = std::string(nameBuffer) + "_left";
        ImGui::Text("  + %s (flipped)", leftName.c_str());
    }

    ImGui::Spacing();

    // Add button
    if (ImGui::Button("Add Animation", ImVec2(-1, 0))) {
        std::string baseName = nameBuffer;

        if (baseName.empty()) {
            UI::SetDebugMessage("[ANIMATION EDITOR] Animation name cannot be empty");
        } else {
            AnimationDefinition def;
            def.name = fullName;
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

                // Auto-create LEFT if RIGHT was added and checkbox is checked
                if (dir == AnimationDirection::RIGHT && autoCreateLeft) {
                    AnimationDefinition leftDef = def;
                    leftDef.name = baseName + "_left";
                    leftDef.direction = AnimationDirection::LEFT;
                    leftDef.isAutoFlipped = true;

                    // Check for duplicate
                    bool leftDuplicate = false;
                    for (const auto& existing : definedAnimations) {
                        if (existing.name == leftDef.name) {
                            leftDuplicate = true;
                            break;
                        }
                    }

                    if (!leftDuplicate) {
                        definedAnimations.push_back(leftDef);
                        UI::SetDebugMessage("[ANIMATION EDITOR] Auto-created: " + leftDef.name + " (flipped)");
                    }
                }
            }
        }
    }
}


void AnimationEditor::RenderDefinedAnimationsList() {
    ImGui::Text("Defined Animations (%d)", (int)definedAnimations.size());
    ImGui::Spacing();

    if (definedAnimations.empty()) {
        ImGui::TextDisabled("No animations defined yet");
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
        bool selectableHovered = ImGui::IsItemHovered();

        // Only change preview if clicking selectable AND not hovering button
        if (clicked && !buttonHovered) {
            previewAnimIndex = i;
            previewFrame = 0;
            previewTimer = 0.0f;
            previewFlipped = anim.isAutoFlipped;
        }

        ImGui::SameLine(30);  // Move back to where the selectable started

        // Show different color for auto-flipped animations
        if (anim.isAutoFlipped) {
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
                "%s (Row %d, %d frames) [FLIP]",
                anim.name.c_str(), anim.row, anim.frameCount);
        } else {
            ImGui::Text("%s (Row %d, %d frames)",
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
        if (ImGui::Button("Clear All")) {
            definedAnimations.clear();
            previewAnimIndex = -1;
        }
    }
}

void AnimationEditor::RenderAnimationPreview() {
    ImGui::Text("Animation Preview");
    ImGui::Spacing();

    if (!textureLoaded || previewAnimIndex < 0 ||
        previewAnimIndex >= (int)definedAnimations.size()) {
        ImGui::TextDisabled("Select an animation to preview");
        return;
    }

    const auto& anim = definedAnimations[previewAnimIndex];

    ImGui::Text("Playing: %s (Frame %d/%d)%s",
                anim.name.c_str(), previewFrame + 1, anim.frameCount,
                anim.isAutoFlipped ? " [FLIPPED]" : "");

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
        uv0 = ImVec2((srcX + srcW) / currentTexture.width,
                     srcY / currentTexture.height);
        uv1 = ImVec2(srcX / currentTexture.width,
                     (srcY + srcH) / currentTexture.height);
    } else {
        uv0 = ImVec2(srcX / currentTexture.width,
                     srcY / currentTexture.height);
        uv1 = ImVec2((srcX + srcW) / currentTexture.width,
                     (srcY + srcH) / currentTexture.height);
    }

    ImGui::Image((ImTextureID)(intptr_t)currentTexture.id, displaySize, uv0, uv1);
}

void AnimationEditor::RenderSaveSection(Engine& engine) {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Save Animations");

    if (targetAsset) {
        ImGui::Text("Target Asset: %s", targetAsset->name.c_str());
        ImGui::Text("Animations: %zu", definedAnimations.size());

        if (definedAnimations.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No animations defined yet");
        }

        ImGui::Separator();

        if (ImGui::Button("Save to Asset", ImVec2(-1, 0))) {
            SaveAnimationsToAsset();
        }

        if (ImGui::Button("Close Editor", ImVec2(-1, 0))) {
            windowOpen = false;
            targetAsset = nullptr;
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                          "No target asset! Open editor from Asset Console.");
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
        columnsInSheet = currentTexture.width / frameWidth;
        rowsInSheet = currentTexture.height / frameHeight;

        // Clamp selected values
        if (selectedRow >= rowsInSheet) {
            selectedRow = std::max(0, rowsInSheet - 1);
        }
        if (selectedFrameCount > columnsInSheet) {
            selectedFrameCount = columnsInSheet;
        }
    } else {
        columnsInSheet = 0;
        rowsInSheet = 0;
    }
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
            def.loop = anim.loop;
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

void AnimationEditor::SaveAnimationsToAsset() {
    if (!targetAsset) {
        UI::SetDebugMessage("[ERROR] No target asset set!");
        return;
    }

    if (definedAnimations.empty()) {
        UI::SetDebugMessage("[WARNING] No animations to save!");
        return;
    }

    if (!textureLoaded) {
        UI::SetDebugMessage("[ERROR] No texture loaded!");
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
    animSet->texture = currentTexture;
    animSet->textureLoaded = textureLoaded;
    animSet->frameWidth = frameWidth;
    animSet->frameHeight = frameHeight;

    // Convert each AnimationDefinition to Animation
    for (const auto& def : definedAnimations) {
        animSet->AddAnimationFromRow(
                def.name,
                def.row,
                def.frameCount,
                def.frameRate,
                def.loop,
                def.trigger,
                def.direction);
    }

    // Mark asset as having animations
    targetAsset->hasAnimations = true;

    UI::SetDebugMessage("[ANIMATION] Saved " + std::to_string(definedAnimations.size()) +
                       " animations to asset: " + targetAsset->name);
}