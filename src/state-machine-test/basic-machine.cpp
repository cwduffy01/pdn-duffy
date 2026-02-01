#include "../../include/state-machine-test/basic-machine.hpp"

BasicMachine::BasicMachine(Device* PDN) : StateMachineRtos(PDN) {

}

BasicMachine::~BasicMachine() {

}

void BasicMachine::populateStateMap() {
    State0* state0 = new State0();
    State1* state1 = new State1();
    State2* state2 = new State2();
    State3* state3 = new State3();

    state0->start(true);
    state1->start(true);
    state2->start(true);
    state3->start(true);

    stateMap[STATE0] = state0;
    stateMap[STATE1] = state1;
    stateMap[STATE2] = state2;
    stateMap[STATE3] = state3;
}