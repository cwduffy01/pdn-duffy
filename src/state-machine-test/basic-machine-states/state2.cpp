#include "device/pdn.hpp"
#include "state-machine-test/basic-machine-states.hpp"
#include "state-machine-test/basic-machine.hpp"
#include <esp_log.h>

State2::State2() : StateRtos(STATE2) {
    count = 0;
}

State2::~State2() {

}

void State2::onStateMounted(Device *PDN) {
    ESP_LOGW(getTaskName(), "Mounted");
}

void State2::onStateLoop(Device *PDN, StateMachineRtos* machine) {
    ESP_LOGW(getTaskName(), "Count = %d", count);

    count++;
    
    if (count > 10) {
        count = 0;

        uint32_t nextState = STATE3;
        machine->sendToQueue(&nextState);
    }

    vTaskDelay(100);
}

void State2::onStateDismounted(Device *PDN) {
    count = 0;
    ESP_LOGW(getTaskName(), "Dismounted");
}