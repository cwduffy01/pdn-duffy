#pragma once

#include "state/connect-state.hpp"
#include "state/state.hpp"
#include "apps/symbol-match/symbol-manager.hpp"
#include "device/device.hpp"
#include "utils/simple-timer.hpp"
#include "wireless/symbol-wireless-manager.hpp"
#include <string>

class HackedPlayersManager;
class FDNConnectWirelessManager;

enum SymbolMatchStateId {
    SELECTION,
    SYMBOL_IDLE,
    MATCH_SUCCESS,
    SYMBOL_MATCH_UPLOAD_PENDING,
    SYMBOL_MATCH_CONNECTION_DETECTED,
    SYMBOL_MATCH_AUTHORIZED_PDN,
    SYMBOL_MATCH_UNAUTHORIZED_PDN,
};

class Selection : public ConnectState {
public:
    explicit Selection(SymbolManager* symbolManager, RemoteDeviceCoordinator* remoteDeviceCoordinator,
                        SymbolWirelessManager* symbolWirelessManager);
    ~Selection();   
    void onStateMounted(Device *FDN) override;
    void onStateLoop(Device *FDN) override;
    void onStateDismounted(Device *FDN) override;
    bool transitionToIdle();
    bool isJackRequired(SerialIdentifier jack) override;

private:
    SymbolManager* symbolManager;
    SymbolWirelessManager* symbolWirelessManager;
    SimpleTimer bufferTimer;
    bool transitionToIdleState = false;
    int bufferInterval = 1 * 1000;

    void onSymbolMatchCommandReceived(SymbolMatchCommand command);
};

class SymbolIdle : public ConnectState {
public:
    explicit SymbolIdle(SymbolManager* symbolManager, RemoteDeviceCoordinator* remoteDeviceCoordinator,
                        SymbolWirelessManager* symbolWirelessManager);
    ~SymbolIdle();
    void onStateMounted(Device *FDN) override;
    void onStateLoop(Device *FDN) override;
    void onStateDismounted(Device *FDN) override;
    bool transitionToSelection();
    bool transitionToMatchSuccess();
    bool transitionToMainMenu();

    bool isJackRequired(SerialIdentifier jack) override;

private:
    void renderSymbolScreen(Device *FDN);

    bool transitionToSelectionState = false;
    bool transitionToMatchSuccessState = false;
    bool transitionToMainMenuApp = false;

    SymbolManager* symbolManager;
    SymbolWirelessManager* symbolWirelessManager;
    /// Set while SYMBOL_IDLE is active; used when ESP-NOW callbacks need the display.
    Device* mountedFdn = nullptr;
    int lastTimeRendered = 0;

    bool leftConnected = false;
    bool rightConnected = false;
    bool blinkToggle = true;
    bool symbolSentLeft = false;
    bool symbolSentRight = false;
};
class MatchSuccess : public ConnectState {
public:
    explicit MatchSuccess(SymbolManager* symbolManager, RemoteDeviceCoordinator* remoteDeviceCoordinator,
        SymbolWirelessManager* symbolWirelessManager);
    ~MatchSuccess();
    void onStateMounted(Device *FDN) override;
    void onStateLoop(Device *FDN) override;
    void onStateDismounted(Device *FDN) override;
    bool transitionToMainMenu();

    bool isJackRequired(SerialIdentifier jack) override;

private:
    SymbolManager* symbolManager;
    SymbolWirelessManager* symbolWirelessManager;

    void renderSymbolScreen(Device *FDN);

    bool toggleBlink = true;

    SimpleTimer bufferTimer;
    int bufferInterval = 3 * 1000;

    SimpleTimer renderTimer;
    int renderInterval = 0.2 * 1000;
};

class SymbolMatchUploadPendingHacksState : public State {
public:
    explicit SymbolMatchUploadPendingHacksState(HackedPlayersManager* hackedPlayersManager);
    ~SymbolMatchUploadPendingHacksState();

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToIdle();

private:
    HackedPlayersManager* hackedPlayersManager;
    SimpleTimer glyphTimer;
    SimpleTimer fallbackTimer;
    bool contentReady = false;
    int pendingCount = 0;
    int completedCount = 0;
    static constexpr int FALLBACK_TIMEOUT_MS = 15000;
};

class SymbolMatchConnectionDetectedState : public ConnectState {
public:
    explicit SymbolMatchConnectionDetectedState(HackedPlayersManager* hackedPlayersManager,
                                                RemoteDeviceCoordinator* remoteDeviceCoordinator);
    ~SymbolMatchConnectionDetectedState();

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool isJackRequired(SerialIdentifier jack) override;
    void receivePdnConnection(const std::string& playerId);
    bool transitionToAuth();
    bool transitionToUnauthorized();
    bool transitionToSelection();

private:
    HackedPlayersManager* hackedPlayersManager;
    SimpleTimer glyphTimer;
    bool contentReady = false;
    bool wasHacked = false;
    std::string pendingPlayerId;
};

class SymbolMatchAuthDetectedState : public ConnectState {
public:
    explicit SymbolMatchAuthDetectedState(RemoteDeviceCoordinator* remoteDeviceCoordinator,
                                          FDNConnectWirelessManager* fdnConnectWirelessManager);
    ~SymbolMatchAuthDetectedState();

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool isJackRequired(SerialIdentifier jack) override;
    bool transitionToSelection();
    bool transitionToMainMenu();

private:
    SimpleTimer glyphTimer;
    SimpleTimer switchTimer;
    bool contentReady = false;
    static constexpr int SWITCH_DELAY_MS = 5000;
    const char* authMessage[2] = {"WELCOME", "ASSET #"};
    FDNConnectWirelessManager* fdnConnectWirelessManager;
};

class SymbolMatchUnauthorizedDetectedState : public ConnectState {
public:
    explicit SymbolMatchUnauthorizedDetectedState(RemoteDeviceCoordinator* remoteDeviceCoordinator);
    ~SymbolMatchUnauthorizedDetectedState();

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool isJackRequired(SerialIdentifier jack) override;
    bool transitionToSelection();
    bool transitionToHacking();

private:
    SimpleTimer glyphTimer;
    SimpleTimer switchTimer;
    bool contentReady = false;
    static constexpr int SWITCH_DELAY_MS = 5000;
    const char* accessDeniedMessage[2] = {"ACCESS", "DENIED"};
};