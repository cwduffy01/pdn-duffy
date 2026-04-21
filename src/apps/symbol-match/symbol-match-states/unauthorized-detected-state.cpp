#include "apps/symbol-match/symbol-match-states.hpp"
#include "device/drivers/logger.hpp"
#include "utils/display-utils.hpp"

#define TAG "SYMBOL_UNAUTH_DETECTED"

SymbolMatchUnauthorizedDetectedState::SymbolMatchUnauthorizedDetectedState(
    RemoteDeviceCoordinator* remoteDeviceCoordinator)
    : ConnectState(remoteDeviceCoordinator, SymbolMatchStateId::SYMBOL_MATCH_UNAUTHORIZED_PDN) {}

SymbolMatchUnauthorizedDetectedState::~SymbolMatchUnauthorizedDetectedState() {}

void SymbolMatchUnauthorizedDetectedState::onStateMounted(Device* PDN) {
    LOG_I(TAG, "Mounted - new player, switching to hacking");
    contentReady = false;
    glyphTimer.setTimer(GLYPH_LOADING_DURATION_MS);
    showLoadingGlyphs(PDN);
}

void SymbolMatchUnauthorizedDetectedState::onStateLoop(Device* PDN) {
    if (isInGlyphLoadingPhase(PDN, glyphTimer)) return;

    if (!contentReady) {
        PDN->getDisplay()
            ->invalidateScreen()
            ->drawText(accessDeniedMessage[0], centeredTextX(accessDeniedMessage[0]), 28)
            ->drawText(accessDeniedMessage[1], centeredTextX(accessDeniedMessage[1]), 44)
            ->render();
        switchTimer.setTimer(SWITCH_DELAY_MS);
        contentReady = true;
    }
}

void SymbolMatchUnauthorizedDetectedState::onStateDismounted(Device* PDN) {
    (void)PDN;
    glyphTimer.invalidate();
    switchTimer.invalidate();
    contentReady = false;
}

bool SymbolMatchUnauthorizedDetectedState::isJackRequired(SerialIdentifier jack) {
    return jack == SerialIdentifier::INPUT_JACK || jack == SerialIdentifier::INPUT_JACK_SECONDARY;
}

bool SymbolMatchUnauthorizedDetectedState::transitionToSelection() {
    return !isConnected();
}

bool SymbolMatchUnauthorizedDetectedState::transitionToHacking() {
    return contentReady && switchTimer.expired();
}
