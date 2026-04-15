#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "raylib.h"

// Defines when an animation should play
enum class AnimationTrigger {
    LOOP,       // Plays continuously (tree swaying, water flowing)
    INPUT,      // Plays when user input is active (player running)
    EVENT,      // Plays once when triggered by game event (taking damage, attacking)
    IDLE        // Plays when no other animation is active
};

// Direction for animations that have directional variants
enum class AnimationDirection {
    NONE,       // No direction (tree swaying, UI animations)
    DOWN,
    UP,
    RIGHT,
    LEFT        // Will use RIGHT frames but flipped horizontally
};

// A single frame within an animation
struct AnimationFrame {
    Rectangle sourceRect;   // The region of the sprite sheet for this frame
    float duration;         // How long this frame displays (in seconds)
};

// A single animation sequence (e.g., "walk_down", "idle", "sway")
struct Animation {
    std::string name;                   // Identifier like "walk_down", "idle_right"
    std::vector<AnimationFrame> frames; // All frames in this animation
    bool loop = true;                   // Whether to loop or play once
    AnimationTrigger trigger = AnimationTrigger::LOOP;
    AnimationDirection direction = AnimationDirection::NONE;
    std::string sourceAssetId;          // Which asset texture this animation should use
    Texture2D sourceTexture = {0};      // Resolved texture (can differ from AnimationSet::texture)
    bool hasSourceTexture = false;
    bool flipHorizontallyAtRuntime = false;

    float GetTotalDuration() const {
        float total = 0.0f;
        for (const auto& frame : frames) {
            total += frame.duration;
        }
        return total;
    }
};

// A complete set of animations from one sprite sheet
struct AnimationSet {
    std::string id;                     // Unique identifier
    std::string textureId;              // Reference to asset in AssetManager
    Texture2D texture = {0};            // The loaded sprite sheet
    bool textureLoaded = false;

    int frameWidth = 32;                // Width of each frame in pixels
    int frameHeight = 32;               // Height of each frame in pixels

    std::unordered_map<std::string, Animation> animations;

    const Animation* GetAnimation(const std::string& name) const {
        auto it = animations.find(name);
        if (it != animations.end()) {
            return &it->second;
        }
        return nullptr;
    }

    Animation* GetAnimation(const std::string& name) {
        auto it = animations.find(name);
        if (it != animations.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Helper to add an animation from row data
    void AddAnimationFromRow(const std::string& name, int row, int frameCount,
                             float frameRate, bool looping = true, bool flipHorizontallyAtRuntime = false,
                             AnimationTrigger trigger = AnimationTrigger::LOOP,
                             AnimationDirection direction = AnimationDirection::NONE,
                             const std::string& sourceAssetId = "") {
        Animation anim;
        anim.name = name;
        anim.loop = looping;
        anim.trigger = trigger;
        anim.direction = direction;
        anim.sourceAssetId = sourceAssetId;
        anim.flipHorizontallyAtRuntime = flipHorizontallyAtRuntime;

        float frameDuration = 1.0f / frameRate;

        for (int i = 0; i < frameCount; i++) {
            AnimationFrame frame{};
            frame.sourceRect = {
                static_cast<float>(i * frameWidth),
                static_cast<float>(row * frameHeight),
                static_cast<float>(frameWidth),
                static_cast<float>(frameHeight)
            };
            frame.duration = frameDuration;
            anim.frames.push_back(frame);
        }

        animations[name] = anim;
    }
};
