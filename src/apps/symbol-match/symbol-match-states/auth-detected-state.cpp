#include "apps/symbol-match/symbol-match-states.hpp"
#include "device/drivers/logger.hpp"
#include "utils/display-utils.hpp"
#include "wireless/fdn-connect-wireless-manager.hpp"

#define TAG "SYMBOL_AUTH_DETECTED"

SymbolMatchAuthDetectedState::SymbolMatchAuthDetectedState(
    RemoteDeviceCoordinator* remoteDeviceCoordinator,
    FDNConnectWirelessManager* fdnConnectWirelessManager)
    : ConnectState(remoteDeviceCoordinator, SymbolMatchStateId::SYMBOL_MATCH_AUTHORIZED_PDN) {
    this->fdnConnectWirelessManager = fdnConnectWirelessManager;
}

SymbolMatchAuthDetectedState::~SymbolMatchAuthDetectedState() {
    fdnConnectWirelessManager = nullptr;
}

void SymbolMatchAuthDetectedState::onStateMounted(Device* PDN) {
    LOG_I(TAG, "Mounted - authorized player, switching to main menu");
    contentReady = false;
    glyphTimer.setTimer(GLYPH_LOADING_DURATION_MS);
    showLoadingGlyphs(PDN);
}

void SymbolMatchAuthDetectedState::onStateLoop(Device* PDN) {
    if (isInGlyphLoadingPhase(PDN, glyphTimer)) {
        return;
    }

    if (!contentReady) {
        PDN->getDisplay()
            ->invalidateScreen()
            ->drawText(authMessage[0], centeredTextX(authMessage[0]), 20)
            ->drawText(authMessage[1], centeredTextX(authMessage[1]), 36)
            ->drawText(fdnConnectWirelessManager->getPeerPlayerId().c_str(),
                       centeredTextX(fdnConnectWirelessManager->getPeerPlayerId().c_str()), 52)
            ->render();
        switchTimer.setTimer(SWITCH_DELAY_MS);
        contentReady = true;
    }
}

void SymbolMatchAuthDetectedState::onStateDismounted(Device* PDN) {
    (void)PDN;
    glyphTimer.invalidate();
    switchTimer.invalidate();
    contentReady = false;
}

bool SymbolMatchAuthDetectedState::isJackRequired(SerialIdentifier jack) {
    return jack == SerialIdentifier::INPUT_JACK || jack == SerialIdentifier::INPUT_JACK_SECONDARY;
}

bool SymbolMatchAuthDetectedState::transitionToSelection() {
    return !isConnected();
}

bool SymbolMatchAuthDetectedState::transitionToMainMenu() {
    return contentReady && switchTimer.expired();
}
