#include "device/pdn.hpp"
#include "state-machine-test/basic-machine-states.hpp"
#include "state-machine-test/basic-machine.hpp"
#include <esp_log.h>

State1::State1() : StateRtos(STATE1) {
    count = 0;
}

State1::~State1() {

}

void State1::onStateMounted(Device *PDN) {
    ESP_LOGW(getTaskName(), "Mounted");
}

void State1::onStateLoop(Device *PDN, StateMachineRtos* machine) {
    ESP_LOGW(getTaskName(), "Count = %d", count);

    count++;
    
    if (count > 10) {
        count = 0;

        uint32_t nextState = STATE2;
        machine->sendToQueue(&nextState);
    }

    vTaskDelay(100);
}

void State1::onStateDismounted(Device *PDN) {
    count = 0;
    ESP_LOGW(getTaskName(), "Dismounted");
}