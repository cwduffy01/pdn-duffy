#include <cstring>

#include "device/drivers/serial-wrapper.hpp"
#include "device/remote-device-coordinator.hpp"
#include "apps/symbol-match/symbol-match-states.hpp"
#include "symbol.hpp"
#include "device/device.hpp"
#include "device/drivers/light-interface.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "SymbolMatch";

namespace {
constexpr uint8_t kRecessCount = 22;  // numRecessLights; physical indices 0–21

/// Recess halves (logical 0–10 / 11–21) with a +2 index shift. Fin: cross to the other physical half —
/// a left jack match lights 4–8, a right jack match lights 0–3 (so the visible fin side matches the
/// opposite edge from the old straight mapping).
LEDState makeMatchSideHalves(const bool leftOn, const bool rightOn) {
    LEDState s;
    s.clear();
    constexpr LEDColor c(100, 160, 255);
    constexpr uint8_t bright = 90;
    if (leftOn) {
        for (uint8_t l = 0; l < 11; ++l) {
            const uint8_t phys = (l + 2U) % kRecessCount;
            s.setRecessLight(phys, c, bright);
        }
        for (uint8_t i = 4; i < 9; ++i) {
            s.setFinLight(i, c, bright);
        }
    }
    if (rightOn) {
        for (uint8_t l = 11; l < 22; ++l) {
            const uint8_t phys = (l + 2U) % kRecessCount;
            s.setRecessLight(phys, c, bright);
        }
        for (uint8_t i = 0; i < 4; ++i) {
            s.setFinLight(i, c, bright);
        }
    }
    return s;
}
}  // namespace

SymbolIdle::SymbolIdle(SymbolManager* symbolManager, RemoteDeviceCoordinator* remoteDeviceCoordinator,
                       SymbolWirelessManager* symbolWirelessManager)
    : ConnectState(remoteDeviceCoordinator, SYMBOL_IDLE) {
    this->symbolManager = symbolManager;
    this->symbolWirelessManager = symbolWirelessManager;
}

SymbolIdle::~SymbolIdle() {
    symbolWirelessManager = nullptr;
}

void SymbolIdle::onStateMounted(Device *FDN) {
    LOG_W(TAG, "SymbolIdle mounted");

    mountedFdn = FDN;

    renderSymbolScreen(FDN);

    symbolWirelessManager->setPacketReceivedCallback([this](const SymbolMatchCommand& command) {
        if (command.command != SMCommand::SEND_SYMBOL) {
            return;
        }

        const SymbolId local = symbolManager->getSymbol(SerialIdentifier::INPUT_JACK)->getSymbolId();
        symbolManager->setMatched(SerialIdentifier::INPUT_JACK, command.symbolId == local);
        if (mountedFdn != nullptr) {
            renderSymbolScreen(mountedFdn);
            syncMatchSideLights(mountedFdn);
        }
    }, SerialIdentifier::INPUT_JACK);

    symbolWirelessManager->setPacketReceivedCallback([this](const SymbolMatchCommand& command) {
        if (command.command != SMCommand::SEND_SYMBOL) {
            return;
        }

        const SymbolId local = symbolManager->getSymbol(SerialIdentifier::INPUT_JACK_SECONDARY)->getSymbolId();
        symbolManager->setMatched(SerialIdentifier::INPUT_JACK_SECONDARY, command.symbolId == local);
        if (mountedFdn != nullptr) {
            renderSymbolScreen(mountedFdn);
            syncMatchSideLights(mountedFdn);
        }
    }, SerialIdentifier::INPUT_JACK_SECONDARY);

    // Send current symbols to all known peers
    for (SerialIdentifier port : {SerialIdentifier::INPUT_JACK_SECONDARY, SerialIdentifier::INPUT_JACK}) {
        const uint8_t* peerMac = remoteDeviceCoordinator->getPeerMac(port);
        if (peerMac != nullptr) {
            symbolWirelessManager->setMacPeer(peerMac);
            if (port == SerialIdentifier::INPUT_JACK) {
                symbolWirelessManager->sendPacket(SMCommand::SEND_SYMBOL, symbolManager->getSymbol(SerialIdentifier::INPUT_JACK)->getSymbolId(), port);
            } else if (port == SerialIdentifier::INPUT_JACK_SECONDARY) {
                symbolWirelessManager->sendPacket(SMCommand::SEND_SYMBOL, symbolManager->getSymbol(SerialIdentifier::INPUT_JACK_SECONDARY)->getSymbolId(), port);
            }
        }
    }

    leftConnected = remoteDeviceCoordinator->getPortStatus(SerialIdentifier::INPUT_JACK) == PortStatus::CONNECTED;
    rightConnected = remoteDeviceCoordinator->getPortStatus(SerialIdentifier::INPUT_JACK_SECONDARY) == PortStatus::CONNECTED;
    const bool leftLeds = symbolManager->isMatched(SerialIdentifier::INPUT_JACK) && symbolSentLeft;
    const bool rightLeds = symbolManager->isMatched(SerialIdentifier::INPUT_JACK_SECONDARY) && symbolSentRight;
    lastSideLightLeft_ = leftLeds;
    lastSideLightRight_ = rightLeds;
    updateMatchSideLights(FDN, leftLeds, rightLeds);
}

void SymbolIdle::onStateLoop(Device *FDN) {
    if (symbolManager->getRefreshTimer()->expired()) {
        transitionToSelectionState = true;
        LOG_W(TAG, "SymbolIdle: transitionToSelectionState = true");
        return;
    } 

    leftConnected = remoteDeviceCoordinator->getPortStatus(SerialIdentifier::INPUT_JACK) == PortStatus::CONNECTED;
    rightConnected = remoteDeviceCoordinator->getPortStatus(SerialIdentifier::INPUT_JACK_SECONDARY) == PortStatus::CONNECTED;

    if (!leftConnected) {
        symbolManager->setMatched(SerialIdentifier::INPUT_JACK, false);
        if (mountedFdn != nullptr) {
            renderSymbolScreen(mountedFdn);
            syncMatchSideLights(mountedFdn);
        }
    }
    if (!rightConnected) {
        symbolManager->setMatched(SerialIdentifier::INPUT_JACK_SECONDARY, false);
        if (mountedFdn != nullptr) {
            renderSymbolScreen(mountedFdn);
            syncMatchSideLights(mountedFdn);
        }
    }

    if (!symbolSentLeft && leftConnected) {
        const uint8_t* peerMac = remoteDeviceCoordinator->getPeerMac(SerialIdentifier::INPUT_JACK);
        if (peerMac != nullptr) {
            symbolWirelessManager->setMacPeer(peerMac);
            symbolWirelessManager->sendPacket(SMCommand::SEND_SYMBOL,
                symbolManager->getSymbol(SerialIdentifier::INPUT_JACK)->getSymbolId(),
                SerialIdentifier::INPUT_JACK);
            symbolSentLeft = true;
        }
    } else if (!leftConnected) {
        symbolSentLeft = false;
    }

    if (!symbolSentRight && rightConnected) {
        const uint8_t* peerMac = remoteDeviceCoordinator->getPeerMac(SerialIdentifier::INPUT_JACK_SECONDARY);
        if (peerMac != nullptr) {
            symbolWirelessManager->setMacPeer(peerMac);
            symbolWirelessManager->sendPacket(SMCommand::SEND_SYMBOL,
                symbolManager->getSymbol(SerialIdentifier::INPUT_JACK_SECONDARY)->getSymbolId(),
                SerialIdentifier::INPUT_JACK_SECONDARY);
            symbolSentRight = true;
        }
    } else if (!rightConnected) {
        symbolSentRight = false;
    }

    if (symbolManager->getRefreshTimer()->isRunning()) {
        int elapsedTime = symbolManager->getRefreshTimer()->getElapsedTime();
        if (elapsedTime - lastTimeRendered >= 500) {
            lastTimeRendered = elapsedTime;
            renderSymbolScreen(FDN);
            blinkToggle = !blinkToggle;
        }
    }

    syncMatchSideLights(FDN);
}

void SymbolIdle::onStateDismounted(Device *FDN) {
    transitionToSelectionState = false;
    LOG_W(TAG, "SymbolIdle dismounted");

    mountedFdn = nullptr;
    lastTimeRendered = 0;

    symbolWirelessManager->clearCallback();

    symbolManager->setMatched(SerialIdentifier::INPUT_JACK, false);
    symbolManager->setMatched(SerialIdentifier::INPUT_JACK_SECONDARY, false);

    for (SerialIdentifier port : {SerialIdentifier::INPUT_JACK_SECONDARY, SerialIdentifier::INPUT_JACK}) {
        const uint8_t* peerMac = remoteDeviceCoordinator->getPeerMac(port);
        if (peerMac != nullptr) {
            symbolWirelessManager->setMacPeer(peerMac);
            symbolWirelessManager->sendPacket(SMCommand::SYMBOLS_REFRESHED, SymbolId::SYMBOL_A, port);
        }
    }

    transitionToMainMenuApp = false;
    FDN->getLightManager()->stopAnimation();
}

void SymbolIdle::updateMatchSideLights(Device* FDN, const bool leftOn, const bool rightOn) {
    FDN->getLightManager()->setStaticLedState(makeMatchSideHalves(leftOn, rightOn));
}

void SymbolIdle::syncMatchSideLights(Device* FDN) {
    const bool l = symbolManager->isMatched(SerialIdentifier::INPUT_JACK) && symbolSentLeft;
    const bool r = symbolManager->isMatched(SerialIdentifier::INPUT_JACK_SECONDARY) && symbolSentRight;
    if (l != lastSideLightLeft_ || r != lastSideLightRight_) {
        lastSideLightLeft_ = l;
        lastSideLightRight_ = r;
        updateMatchSideLights(FDN, l, r);
    }
}

void SymbolIdle::renderSymbolScreen(Device *FDN) {
    FDN->getDisplay()->invalidateScreen();

    // render symbol glyphs
    if (symbolManager->isMatched(SerialIdentifier::INPUT_JACK)) {
        FDN->getDisplay()->whiteScreenLeftHalf();
    }

    if (symbolManager->isMatched(SerialIdentifier::INPUT_JACK_SECONDARY)) {
        FDN->getDisplay()->whiteScreenRightHalf();
    }

    // Half-screen fills reset draw color / font mode; SYMBOL_GLYPH uses XOR (draw color 2) + transparent font
    FDN->getDisplay()->setGlyphMode(FontMode::SYMBOL_GLYPH);

    if (symbolManager->isMatched(SerialIdentifier::INPUT_JACK) || !leftConnected || blinkToggle) {
        FDN->getDisplay()->renderGlyph(symbolManager->getSymbolGlyph(SerialIdentifier::INPUT_JACK), 24, 40);
    } 

    if (symbolManager->isMatched(SerialIdentifier::INPUT_JACK_SECONDARY) || !rightConnected || blinkToggle) {
        FDN->getDisplay()->renderGlyph(symbolManager->getSymbolGlyph(SerialIdentifier::INPUT_JACK_SECONDARY), 72, 40);
    }

    FDN->getDisplay()->setGlyphMode(FontMode::TEXT_INVERTED_LARGE);

    int timeLeft = symbolManager->getTimeLeftToRefresh();
    
    int minutes = timeLeft / 60000;
    int seconds = (timeLeft % 60000) / 1000;

    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
    FDN->getDisplay()->drawText(buffer, 40, 64);

    FDN->getDisplay()->render();
}

bool SymbolIdle::transitionToSelection() {
    return transitionToSelectionState;
}

bool SymbolIdle::transitionToMatchSuccess() {
    return symbolManager->isMatched(SerialIdentifier::INPUT_JACK)
        && symbolManager->isMatched(SerialIdentifier::INPUT_JACK_SECONDARY) && symbolSentLeft
        && symbolSentRight;
}

bool SymbolIdle::transitionToMainMenu() {
    // if a connected device has already matches on this FDN (check storage!!)
    return transitionToMainMenuApp;
}

bool SymbolIdle::isJackRequired(SerialIdentifier jack) {
    (void)jack;
    return true;
}
