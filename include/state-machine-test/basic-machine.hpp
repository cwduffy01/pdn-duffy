#ifndef BASIC_MACHINE_HPP
#define BASIC_MACHINE_HPP

#include "device.hpp"
#include "state-machine-rtos.hpp"
#include "basic-machine-states.hpp"


class BasicMachine : public StateMachineRtos {
public:
    BasicMachine(Device *PDN);

    ~BasicMachine();

    void populateStateMap() override;

protected:
    const char* getTaskName() const override { return "BasicMachine"; }
    uint32_t getStackSize() const override { return 4096; }
    UBaseType_t getPriority() const override { return 5; }
    UBaseType_t getQueueLength() const override { return 5; }
    UBaseType_t getQueueItemSize() const override { return sizeof(int); }

private:

};

#endif // BASIC_MACHINE_HPP