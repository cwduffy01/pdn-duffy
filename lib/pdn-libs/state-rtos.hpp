#pragma once

#include <functional>
#include <set>
#include <utility>
#include <vector>
#include <string>

#include "device.hpp"
#include "rtos-module.hpp"

class State;
class StateMachineRtos;  // Forward declaration instead of include

//StateId is a simple wrapper in case we eventually need to add more data to identify a state.
struct StateRtosId {
    explicit StateRtosId(int stateid) {
        id = stateid;
    }

    int id;
};

// /*
//  * A State transition is a tuple that holds a condition as well as
//  * the state which the condition, when valid, will be transitioned to.
//  *
//  *  condition: A function that returns a boolean signifying the state machine
//  *  should transition to the new state.
//  *
//  *  nextState: A pointer to the next valid state.
//  */
// class StateTransition {
// public:
//     // Constructor
//     StateTransition(std::function<bool()> condition, State *nextState)
//         : condition(condition), nextState(nextState) {
//     };

//     // Method to check if the transition condition is met
//     bool isConditionMet() const {
//         return condition();
//     };

//     // Getter for the next state
//     State *getNextState() const {
//         return nextState;
//     };

//     std::function<bool()> condition; // Function pointer that returns true based on the global state
//     State *nextState; // Pointer to the next state
// };


class StateRtos : public RtosModule {
public:
    virtual ~StateRtos() {
    //     // validStringMessages.erase(validStringMessages.begin(), validStringMessages.end());
    //     // responseStringMessages.erase(responseStringMessages.begin(), responseStringMessages.end());
    //     // transitions.erase(transitions.begin(), transitions.end());
    };

    explicit StateRtos(int stateId): name(stateId), _device(nullptr), _machine(nullptr) {
    }

    /**
     * @brief Set the context for this state (Device and StateMachine).
     * Called by the state machine when mounting this state.
     */
    void setContext(Device* device, StateMachineRtos* machine) {
        _device = device;
        _machine = machine;
    }

    // void addTransition(StateTransition *transition) {
    //     transitions.push_back(transition);
    // };

    // State *checkTransitions() {
    //     for (StateTransition *transition: transitions) {
    //         if (transition->isConditionMet()) {
    //             return transition->nextState;
    //         }
    //     }
    //     return nullptr;
    // };

    int getStateId() const { return name.id; };

    virtual void onStateMounted(Device *PDN) {
    };

    virtual void onStateLoop(Device *PDN, StateMachineRtos *machine) {
    };

    virtual void onStateDismounted(Device *PDN) {
    };

    virtual bool isTerminalState() {
        return false;
    }

    // /*
    //  * Creates a set of valid String messages for this state.
    //  * Any message that is received that is *not* in this set
    //  * will be discarded.
    //  */
    // virtual void registerValidMessages(std::vector<const string *> msgs) {
    //     for (auto msg: msgs) {
    //         validStringMessages.insert(*msg);
    //     }
    // };

    // /*
    //  * This method registers valid messages that can be *sent* during this state's lifecycle.
    //  */
    // virtual void registerResponseMessage(std::vector<const string *> msgs) {
    //     for (int i = 0; i < msgs.size(); i++) {
    //         responseStringMessages.push_back(*msgs.at(i));
    //     }
    // };

    // //Checks if the currently received String message is a part of the set of valid messages.
    // bool isMessageValidForState(string *msg) {
    //     bool isValid = validStringMessages.find(*msg) != validStringMessages.end();
    //     return isValid;
    // }

    // /*
    //  * This message will consume the incoming Serial stream until a valid
    //  * message is found, at which point, that message will be returned.
    //  */
    // string *waitForValidMessage(Device *PDN) {
    //     while (PDN->commsAvailable()) {
    //         if (!isMessageValidForState(PDN->peekComms())) {
    //             PDN->readString();
    //         } else {
    //             return new string(PDN->readString());
    //         }
    //     }
    //     return nullptr;
    // }

protected:
    // std::set<string> validStringMessages;
    // std::vector<string> responseStringMessages;
    // std::vector<StateTransition *> transitions;

    /// Context pointers set by the state machine
    Device* _device;
    StateMachineRtos* _machine;

    /**
     * @brief Default task function implementation that loops onStateLoop().
     * Override this if you need custom task behavior.
     */
    void taskFunction() override {
        for (;;) {
            onStateLoop(_device, _machine);
            vTaskDelay(1);  // Yield to other tasks
        }
    }

    uint32_t getStackSize() const override { return 4096; }
    UBaseType_t getPriority() const override { return 1; }
    UBaseType_t getQueueLength() const override { return 5; }
    UBaseType_t getQueueItemSize() const override { return sizeof(int); }

private:
    StateRtosId name;
};
