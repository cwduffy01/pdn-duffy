#pragma once

#include "device/device.hpp"
#include "state/state-machine.hpp"
#include "apps/symbol-match/symbol-match-states.hpp"
#include "apps/symbol-match/symbol-manager.hpp"
#include "utils/simple-timer.hpp"
#include <string>

class SymbolWirelessManager;
class RemotePlayerManager;
class HackedPlayersManager;
class FDNConnectWirelessManager;

class SymbolMatch : public StateMachine {
public:
    SymbolMatch(Device* FDN, SymbolWirelessManager* symbolWirelessManager, RemotePlayerManager* remotePlayerManager,
                HackedPlayersManager* hackedPlayersManager, FDNConnectWirelessManager* fdnConnectWirelessManager);
    ~SymbolMatch();

    void populateStateMap() override;

    void onStateMounted(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;

private:
    bool shouldTransitionToUploadPending();
    void startIdleLightAnimation(Device* PDN);
    void triggerNearbyDeviceAnimation(Device* PDN, int rssi);
    void updateNearbyDeviceAnimation(Device* PDN);

    RemoteDeviceCoordinator* remoteDeviceCoordinator;
    SymbolManager* symbolManager;
    SymbolWirelessManager* symbolWirelessManager;
    RemotePlayerManager* remotePlayerManager;
    HackedPlayersManager* hackedPlayersManager;
    FDNConnectWirelessManager* fdnConnectWirelessManager;
    bool connectionResolved = false;
    std::string pendingConnectedPlayerId;

    SimpleTimer uploadCheckTimer;
    SimpleTimer nearbyAnimationTimer;
    bool nearbyAnimationActive = false;

    struct NearbyAnimationProfile {
        int timeoutMs;
        uint8_t intensity;
    };

    static constexpr int UPLOAD_CHECK_INTERVAL_MS = 5 * 60 * 1000;
    static constexpr int kUploadPendingStateIndex = 3;
    static constexpr int kConnectionDetectedStateIndex = 4;
    static constexpr int STRONG_RSSI_THRESHOLD = -50;
    static constexpr int MEDIUM_RSSI_THRESHOLD = -60;
};