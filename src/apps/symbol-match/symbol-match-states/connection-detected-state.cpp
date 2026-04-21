#include "apps/symbol-match/symbol-match-states.hpp"
#include "apps/hacking/hacked-players-manager.hpp"
#include "device/drivers/logger.hpp"
#include "utils/display-utils.hpp"

#define TAG "SYMBOL_CONN_DETECTED"

SymbolMatchConnectionDetectedState::SymbolMatchConnectionDetectedState(
    HackedPlayersManager* hackedPlayersManager,
    RemoteDeviceCoordinator* remoteDeviceCoordinator)
    : ConnectState(remoteDeviceCoordinator, SymbolMatchStateId::SYMBOL_MATCH_CONNECTION_DETECTED) {
    this->hackedPlayersManager = hackedPlayersManager;
}

SymbolMatchConnectionDetectedState::~SymbolMatchConnectionDetectedState() {
    hackedPlayersManager = nullptr;
}

void SymbolMatchConnectionDetectedState::receivePdnConnection(const std::string& playerId) {
    pendingPlayerId = playerId;
}

void SymbolMatchConnectionDetectedState::onStateMounted(Device* PDN) {
    LOG_I(TAG, "Mounted - player: %s", pendingPlayerId.c_str());
    contentReady = false;
    glyphTimer.setTimer(GLYPH_LOADING_DURATION_MS);
    showLoadingGlyphs(PDN);
}

void SymbolMatchConnectionDetectedState::onStateLoop(Device* PDN) {
    if (isInGlyphLoadingPhase(PDN, glyphTimer)) return;

    if (!contentReady) {
        wasHacked = hackedPlayersManager->hasPlayerHacked(pendingPlayerId);
        LOG_I(TAG, "Player %s - hacked: %s", pendingPlayerId.c_str(), wasHacked ? "yes" : "no");
        PDN->getDisplay()->invalidateScreen()->render();
        contentReady = true;
    }
}

void SymbolMatchConnectionDetectedState::onStateDismounted(Device* PDN) {
    (void)PDN;
    glyphTimer.invalidate();
    contentReady = false;
    wasHacked = false;
    pendingPlayerId.clear();
}

bool SymbolMatchConnectionDetectedState::isJackRequired(SerialIdentifier jack) {
    return jack == SerialIdentifier::INPUT_JACK || jack == SerialIdentifier::INPUT_JACK_SECONDARY;
}

bool SymbolMatchConnectionDetectedState::transitionToAuth() {
    return contentReady && wasHacked;
}

bool SymbolMatchConnectionDetectedState::transitionToUnauthorized() {
    return contentReady && !wasHacked;
}

bool SymbolMatchConnectionDetectedState::transitionToSelection() {
    return !isConnected();
}
