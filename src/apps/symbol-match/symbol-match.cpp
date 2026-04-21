#include "apps/symbol-match/symbol-match.hpp"
#include "apps/app-ids.hpp"
#include "apps/idle/states/upload-pending-hacks-state.hpp"
#include "wireless/remote-player-manager.hpp"
#include "apps/hacking/hacked-players-manager.hpp"
#include "device/drivers/light-interface.hpp"
#include "device/drivers/logger.hpp"
#include <algorithm>

namespace {
constexpr uint8_t kIdleAnimationSpeed = 20;
constexpr uint8_t kNearbyAnimationMinSpeed = 8;
static const char* TAG = "SymbolMatch";
}

SymbolMatch::SymbolMatch(Device* FDN, SymbolWirelessManager* symbolWirelessManager,
                         RemotePlayerManager* remotePlayerManager, HackedPlayersManager* hackedPlayersManager)
    : StateMachine(SYMBOL_MATCH_APP_ID) {
    this->remoteDeviceCoordinator = FDN->getRemoteDeviceCoordinator();
    this->symbolManager = new SymbolManager();
    this->symbolWirelessManager = symbolWirelessManager;
    this->remotePlayerManager = remotePlayerManager;
    this->hackedPlayersManager = hackedPlayersManager;
}

SymbolMatch::~SymbolMatch() {
    remoteDeviceCoordinator = nullptr;
    delete symbolManager;
    symbolWirelessManager = nullptr;
    remotePlayerManager = nullptr;
    hackedPlayersManager = nullptr;
}

void SymbolMatch::onStateMounted(Device* PDN) {
    // Avoid stale nearby-device events from firing immediately on app entry.
    remotePlayerManager->consumePacketReceived();
    nearbyAnimationTimer.invalidate();
    nearbyAnimationActive = false;
    startIdleLightAnimation(PDN);
    LOG_I(TAG, "LED: baseline IDLE animation started (app mount)");

    StateMachine::onStateMounted(PDN);
}

void SymbolMatch::onStateDismounted(Device* PDN) {
    uploadCheckTimer.invalidate();
    nearbyAnimationTimer.invalidate();
    nearbyAnimationActive = false;
    PDN->getLightManager()->stopAnimation();
    StateMachine::onStateDismounted(PDN);
}

bool SymbolMatch::shouldTransitionToUploadPending() {
    return uploadCheckTimer.expired() && !hackedPlayersManager->getPendingUploads().empty();
}

void SymbolMatch::startIdleLightAnimation(Device* PDN) {
    AnimationConfig cfg{};
    cfg.type = AnimationType::IDLE;
    cfg.loop = true;
    cfg.speed = kIdleAnimationSpeed;
    PDN->getLightManager()->startAnimation(cfg);
}

void SymbolMatch::triggerNearbyDeviceAnimation(Device* PDN, int rssi) {
    NearbyAnimationProfile profile{};
    const char* profileName = "WEAK";
    if (rssi > STRONG_RSSI_THRESHOLD) {
        profile = {5000, 255};
        profileName = "STRONG";
    } else if (rssi > MEDIUM_RSSI_THRESHOLD) {
        profile = {2500, 150};
        profileName = "MEDIUM";
    } else {
        profile = {2500, 80};
    }

    LOG_I(TAG,
          "LED: PLAYER_DETECTED animation — rssi=%d profile=%s timeoutMs=%d intensity=%u",
          rssi,
          profileName,
          profile.timeoutMs,
          static_cast<unsigned>(profile.intensity));

    LEDState initialState;
    for (uint8_t i = 0; i < 23; i++) {
        initialState.setRecessLight(i, LEDColor(), profile.intensity);
    }
    for (uint8_t i = 0; i < 9; i++) {
        initialState.setFinLight(i, LEDColor(), profile.intensity);
    }

    const uint8_t frameSpeed = static_cast<uint8_t>(
        std::max(static_cast<int>(kNearbyAnimationMinSpeed), (255 - profile.intensity) / 7));

    AnimationConfig cfg{};
    cfg.type = AnimationType::PLAYER_DETECTED;
    cfg.loop = true;
    cfg.speed = frameSpeed;
    cfg.curve = EaseCurve::EASE_IN_OUT;
    cfg.initialState = initialState;
    cfg.durationMs = static_cast<uint32_t>(profile.timeoutMs);
    PDN->getLightManager()->startAnimation(cfg);

    nearbyAnimationTimer.setTimer(profile.timeoutMs);
    nearbyAnimationActive = true;
}

void SymbolMatch::updateNearbyDeviceAnimation(Device* PDN) {
    if (!nearbyAnimationActive) {
        return;
    }
    if (nearbyAnimationTimer.expired()) {
        nearbyAnimationTimer.invalidate();
        nearbyAnimationActive = false;
        startIdleLightAnimation(PDN);
        LOG_I(TAG, "LED: PLAYER_DETECTED finished — restored baseline IDLE animation");
    }
}

void SymbolMatch::onStateLoop(Device* PDN) {
    remotePlayerManager->Update();
    if (remotePlayerManager->consumePacketReceived()) {
        triggerNearbyDeviceAnimation(PDN, remotePlayerManager->getLastRssi());
    }
    updateNearbyDeviceAnimation(PDN);

    if (currentState && currentState->getStateId() == SymbolMatchStateId::SELECTION) {
        if (!uploadCheckTimer.isRunning()) {
            uploadCheckTimer.setTimer(UPLOAD_CHECK_INTERVAL_MS);
        }
        if (shouldTransitionToUploadPending()) {
            const auto pending = hackedPlayersManager->getPendingUploads();
            LOG_I(TAG,
                  "Upload: entering sync state — %u pending hack record(s) queued for server",
                  static_cast<unsigned>(pending.size()));
            skipToState(PDN, kUploadPendingStateIndex);
            uploadCheckTimer.invalidate();
            return;
        }
    } else {
        uploadCheckTimer.invalidate();
    }

    StateMachine::onStateLoop(PDN);
}

void SymbolMatch::populateStateMap() {

    Selection* selection = new Selection(symbolManager, remoteDeviceCoordinator, symbolWirelessManager);
    SymbolIdle* symbolIdle = new SymbolIdle(symbolManager, remoteDeviceCoordinator, symbolWirelessManager);
    MatchSuccess* matchSuccess = new MatchSuccess(symbolManager, remoteDeviceCoordinator, symbolWirelessManager);
    auto* uploadPending = new UploadPendingHacksState(hackedPlayersManager);

    uploadPending->addTransition(
        new StateTransition(
            std::bind(&UploadPendingHacksState::transitionToIdle, uploadPending),
            selection));

    selection->addTransition(
        new StateTransition(
            std::bind(&Selection::transitionToIdle, selection),
            symbolIdle));

    symbolIdle->addTransition(
        new StateTransition(
            std::bind(&SymbolIdle::transitionToSelection, symbolIdle),
            selection));

    symbolIdle->addAppTransition(
        std::bind(&SymbolIdle::transitionToMainMenu, symbolIdle), StateId(MAIN_MENU_APP_ID));

    symbolIdle->addTransition(
        new StateTransition(
            std::bind(&SymbolIdle::transitionToMatchSuccess, symbolIdle),
            matchSuccess));

    matchSuccess->addAppTransition(
        std::bind(&MatchSuccess::transitionToMainMenu, matchSuccess), StateId(MAIN_MENU_APP_ID));

    stateMap.push_back(selection);
    stateMap.push_back(symbolIdle);
    stateMap.push_back(matchSuccess);
    stateMap.push_back(uploadPending);
}
