#pragma once

#include "device/device.hpp"
#include "state/state-machine.hpp"
#include "apps/symbol-match/symbol-match-states.hpp"
#include "apps/symbol-match/symbol-manager.hpp"

class SymbolWirelessManager;

class SymbolMatch : public StateMachine {
public:
    SymbolMatch(Device* FDN, SymbolWirelessManager* symbolWirelessManager);
    ~SymbolMatch();

    void populateStateMap() override;

private:
    RemoteDeviceCoordinator* remoteDeviceCoordinator;
    SymbolManager* symbolManager;
    SymbolWirelessManager* symbolWirelessManager;
};