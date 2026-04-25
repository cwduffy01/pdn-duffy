#pragma once

#include "animation-base.hpp"
#include "utils/simple-timer.hpp"
#include <algorithm>
#include <cstdint>

// Bounty (orange/red) celebration — recess + fin, aligned with FDN `LEDState` / `LightManager`.
// Palette matches `bountyIdleLEDColors` in quickdraw-resources.hpp (avoid including that TU here).
class BountyWinAnimation : public AnimationBase {
public:
    BountyWinAnimation() : AnimationBase(), phase_(0) {}

protected:
    void onInit() override {
        phase_ = 0;
        if (config_.speed == 0) {
            frameTimer_.setTimer(24);
        }
        currentState_.clear();
    }

    LEDState onAnimate() override {
        ++phase_;
        const uint8_t maxBright = 220;
        for (uint8_t i = 0; i < kNumRecess; ++i) {
            const uint8_t c = static_cast<uint8_t>((i + (phase_ / 3)) & 7);
            const uint8_t t = getEasingValue(
                static_cast<uint8_t>((i * 11 + phase_ * 4) & 255), EaseCurve::EASE_IN_OUT);
            const uint8_t br = static_cast<uint8_t>((maxBright * t) / 255);
            currentState_.setRecessLight(i, kBountyWinPalette[c], std::max<uint8_t>(40, br));
        }
        for (uint8_t i = 0; i < kNumFin; ++i) {
            const uint8_t c = static_cast<uint8_t>((7 - (i + phase_ / 2)) & 7);
            const uint8_t t = getEasingValue(
                static_cast<uint8_t>((i * 19 + phase_ * 3) & 255), EaseCurve::EASE_IN_OUT);
            const uint8_t br = static_cast<uint8_t>((maxBright * t) / 255);
            currentState_.setFinLight(i, kBountyWinPalette[c], std::max<uint8_t>(40, br));
        }
        return currentState_;
    }

private:
    static constexpr uint8_t kNumRecess = 22;
    static constexpr uint8_t kNumFin = 9;
    uint32_t phase_{};

    static constexpr LEDColor kBountyWinPalette[8] = {
        LEDColor(255, 2, 1),
        LEDColor(237, 75, 0),
        LEDColor(255, 51, 0),
        LEDColor(222, 97, 7),
        LEDColor(255, 2, 1),
        LEDColor(237, 75, 0),
        LEDColor(255, 51, 0),
        LEDColor(222, 97, 7),
    };
};
