#include "device/pdn.hpp"
#include "state-machine-test/basic-machine-states.hpp"
#include "state-machine-test/basic-machine.hpp"
#include <esp_log.h>

State3::State3() : StateRtos(STATE3) {
    count = 0;
}

State3::~State3() {

}

void State3::onStateMounted(Device *PDN) {
    ESP_LOGW(getTaskName(), "Mounted");
}

void State3::onStateLoop(Device *PDN, StateMachineRtos* machine) {
    ESP_LOGW(getTaskName(), "Count = %d", count);

    count++;
    
    if (count > 10) {
        count = 0;

        uint32_t nextState = STATE0;
        machine->sendToQueue(&nextState);
    }

    vTaskDelay(100);
}

void State3::onStateDismounted(Device *PDN) {
    ESP_LOGW(getTaskName(), "Dismounted");
}