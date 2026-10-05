#include "slam_lasting.h"
#include <algorithm>
#include <cmath>

namespace dingosdk::slam {
namespace {
float length(const Vec3& v) { return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
}

void LastingInjuries::step(const Frame& frame, std::uint64_t entity, std::uint64_t now) {
    if (!frame.valid) return;
    if (entity_ && entity != entity_) heal("a new skater");
    entity_ = entity;
    // The same jump test the challenge uses for a teleport: further than two metres, and
    // further than three frames at the fastest body's speed.
    if (previous_valid_) {
        float speed{};
        for (std::size_t i = 0; i < frame.body_count; ++i) speed = std::max(speed, length(frame.bodies[i].velocity));
        Vec3 moved{};
        for (std::size_t i = 0; i < 3; ++i) moved[i] = frame.center[i] - previous_.center[i];
        if (length(moved) > std::max(2.f, speed * frame.dt * 3.f)) heal("respawn");
    }
    previous_ = frame;
    previous_valid_ = true;

    tracker_.step(frame, now);
    const auto& injuries = tracker_.result().injuries;
    for (std::size_t r = 0; r < region_count; ++r)
        if (injuries[r].fractured && !broken_[r]) {
            broken_[r] = true;
            changed_ = true;
        }
}

void LastingInjuries::heal(std::string_view why) {
    if (std::ranges::any_of(broken_, [](bool b) { return b; })) changed_ = true;
    broken_ = {};
    tracker_.reset();
    previous_valid_ = false;
    last_heal_ = std::string(why);
}

std::vector<std::string_view> LastingInjuries::wanted() const {
    const auto broken = [&](Region r) { return broken_[static_cast<std::size_t>(r)]; };
    std::vector<std::string_view> out;
    const int legs = static_cast<int>(broken(Region::left_leg)) + static_cast<int>(broken(Region::right_leg));
    if (legs == 1) out.push_back(lasting_leg_preset);
    if (legs == 2) out.push_back(lasting_legs_preset);
    if (broken(Region::left_arm) || broken(Region::right_arm)) out.push_back(lasting_arm_preset);
    if (broken(Region::head)) out.push_back(lasting_head_preset);
    out.push_back(lasting_jam_preset);
    return out;
}

std::string LastingInjuries::summary() const {
    std::string out;
    for (std::size_t r = 0; r < region_count; ++r)
        if (broken_[r] && static_cast<Region>(r) != Region::torso) {
            if (!out.empty()) out += ", ";
            out += region_name(static_cast<Region>(r));
        }
    return out.empty() ? "none" : out;
}
}
