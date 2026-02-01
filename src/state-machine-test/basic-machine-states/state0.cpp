#include "device/pdn.hpp"
#include "state-machine-test/basic-machine-states.hpp"
#include "state-machine-test/basic-machine.hpp"
#include <esp_log.h>

State0::State0() : StateRtos(STATE0) {
    count = 0;
}

State0::~State0() {

}

void State0::onStateMounted(Device *PDN) {
    ESP_LOGW(getTaskName(), "Mounted");
}

void State0::onStateLoop(Device *PDN, StateMachineRtos* machine) {
    ESP_LOGW(getTaskName(), "Count = %d", count);

    count++;
    
    if (count > 10) {
        count = 0;

        uint32_t nextState = STATE1;
        machine->sendToQueue(&nextState);
    }

    vTaskDelay(100);
}

void State0::onStateDismounted(Device *PDN) {
    count = 0;
    ESP_LOGW(getTaskName(), "Dismounted");
}