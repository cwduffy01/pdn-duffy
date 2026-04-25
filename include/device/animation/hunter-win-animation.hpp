#pragma once

#include "animation-base.hpp"
#include "utils/simple-timer.hpp"
#include <algorithm>
#include <cstdint>

// Hunter (green) celebration — recess + fin strips, aligned with FDN `LEDState` / `LightManager`.
// Palette matches `hunterIdleLEDColors` in quickdraw-resources.hpp (avoid including that TU here).
class HunterWinAnimation : public AnimationBase {
public:
    HunterWinAnimation() : AnimationBase(), phase_(0) {}

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
            currentState_.setRecessLight(i, kHunterWinPalette[c], std::max<uint8_t>(40, br));
        }
        for (uint8_t i = 0; i < kNumFin; ++i) {
            const uint8_t c = static_cast<uint8_t>((7 - (i + phase_ / 2)) & 7);
            const uint8_t t = getEasingValue(
                static_cast<uint8_t>((i * 19 + phase_ * 3) & 255), EaseCurve::EASE_IN_OUT);
            const uint8_t br = static_cast<uint8_t>((maxBright * t) / 255);
            currentState_.setFinLight(i, kHunterWinPalette[c], std::max<uint8_t>(40, br));
        }
        return currentState_;
    }

private:
    static constexpr uint8_t kNumRecess = 22;
    static constexpr uint8_t kNumFin = 9;
    uint32_t phase_{};

    static constexpr LEDColor kHunterWinPalette[8] = {
        LEDColor(0, 255, 0),
        LEDColor(0, 237, 75),
        LEDColor(0, 200, 100),
        LEDColor(0, 180, 180),
        LEDColor(0, 255, 0),
        LEDColor(0, 237, 75),
        LEDColor(0, 200, 100),
        LEDColor(0, 180, 180),
    };
};
