#pragma once
// Lasting injuries: a bone the Hall of Meat model fractures stays broken while the skater
// keeps skating, and only a respawn (any teleport of the skater, a level change or a new
// skater) heals it. Each injured region maps to one Trainer preset (trainer_presets.cpp)
// that changes how the skater rides; "Jam" is on whenever the feature is.
// The model is pure: the runtime steps it on the skeleton hook and applies the presets it
// asks for on the game thread.
#include "slam_visuals.h"
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dingosdk::slam {
inline constexpr std::string_view lasting_leg_preset = "Injury: Broken Leg";
inline constexpr std::string_view lasting_legs_preset = "Injury: Broken Legs";
inline constexpr std::string_view lasting_arm_preset = "Injury: Broken Arm";
inline constexpr std::string_view lasting_head_preset = "Injury: Concussion";
inline constexpr std::string_view lasting_jam_preset = "Jam";
// Every preset the feature may switch on, in the order they are applied.
inline constexpr std::array<std::string_view, 5> lasting_presets{
    lasting_leg_preset, lasting_legs_preset, lasting_arm_preset, lasting_head_preset, lasting_jam_preset};

class LastingInjuries {
public:
    // One verified skeleton sample. `entity` identifies the local skater.
    void step(const Frame& frame, std::uint64_t entity, std::uint64_t now);
    void heal(std::string_view why);
    const std::array<bool, region_count>& broken() const noexcept { return broken_; }
    // The presets the current injuries call for (Jam included).
    std::vector<std::string_view> wanted() const;
    // "left leg, right arm" or "none"; the reason of the last heal.
    std::string summary() const;
    const std::string& last_heal() const noexcept { return last_heal_; }
    // A new fracture since the last call (for the log).
    bool take_changed() noexcept { const bool c = changed_; changed_ = false; return c; }
private:
    FreeplayXray tracker_;
    std::array<bool, region_count> broken_{};
    Frame previous_{};
    bool previous_valid_{};
    std::uint64_t entity_{};
    std::string last_heal_;
    bool changed_{};
};
}
