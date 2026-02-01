#ifndef STATE_MACHINE_RTOS_HPP
#define STATE_MACHINE_RTOS_HPP

#include "rtos-module.hpp"

#include <map>
#include <vector>
#include "device.hpp"
#include "state-rtos.hpp"

#include <esp_log.h>

class StateMachineRtos : public RtosModule {
public:
    StateMachineRtos(Device *PDN) {
        this->PDN = PDN;
    }

    virtual ~StateMachineRtos() {
        for (auto [key, val]: stateMap) {
            delete val;
        }
    }

    void initialize() {
        populateStateMap();
        currentState = stateMap[0];
        currentState->setContext(PDN, this);  // Set context before mounting
        currentState->onStateMounted(PDN);
        currentState->resume();
    }

    virtual void populateStateMap() = 0;

    // void checkStateTransitions() {
    //     newState = currentState->checkTransitions();
    //     stateChangeReady = (newState != nullptr);
    // };

    // void commitState() {
    //     currentState->onStateDismounted(PDN);

    //     currentState = newState;
    //     stateChangeReady = false;
    //     newState = nullptr;

    //     PDN->onStateChange();

    //     currentState->onStateMounted(PDN);
    // };

    // void loop() {
    //     currentState->onStateLoop(PDN);
    //     // checkStateTransitions();
    //     // if (stateChangeReady) {
    //     //     commitState();
    //     // }
    // }

    StateRtos *getCurrentState() {
        return currentState;
    }

protected:
    // initial state is 0 in the list here
    // std::vector<StateRtos *> stateMap;

    std::map<uint32_t, StateRtos *> stateMap;

    // bool stateChangeReady = false;

    StateRtos *newState = nullptr;
    StateRtos *currentState = nullptr;


    void taskFunction() override {
        for (;;) {
            uint32_t val;
            receiveFromQueue(&val);

            currentState->suspend();
            currentState->onStateDismounted(PDN);

            ESP_LOGW(getTaskName(), "Transitioning to %d", val);

            newState = stateMap[val];
            currentState = newState;
            newState = nullptr;

            currentState->setContext(PDN, this);  // Set context before mounting
            currentState->onStateMounted(PDN);
            currentState->resume();
        }
    }

private:
    Device *PDN;
};

#endif // STATE_MACHINE_RTOS_HPP