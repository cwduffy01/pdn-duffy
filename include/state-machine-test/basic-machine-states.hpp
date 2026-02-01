#pragma once

#include "state-rtos.hpp"

enum BasicMachineStateId {
    STATE0 = 0,
    STATE1 = 1,
    STATE2 = 2,
    STATE3 = 3,
};

class State0 : public StateRtos {
public:
    State0();
    ~State0();

protected:
    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN, StateMachineRtos* machine) override;
    void onStateDismounted(Device *PDN) override;

    // taskFunction() is inherited from StateRtos - it loops onStateLoop() automatically
    const char* getTaskName() const override { return "State0"; }

private:
    int count;
};

class State1 : public StateRtos {
public:
    State1();
    ~State1();

protected:
    const char* getTaskName() const override { return "State1"; }

    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN, StateMachineRtos* machine) override;
    void onStateDismounted(Device *PDN) override;

private:
    int count;
};

class State2 : public StateRtos {
public:
    State2();
    ~State2();

protected:
    const char* getTaskName() const override { return "State2"; }

    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN, StateMachineRtos* machine) override;
    void onStateDismounted(Device *PDN) override;

private:
    int count;
};

class State3 : public StateRtos {
public:
    State3();
    ~State3();

protected:
    const char* getTaskName() const override { return "State3"; }

    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN, StateMachineRtos* machine) override;
    void onStateDismounted(Device *PDN) override;

private:
    int count;
};