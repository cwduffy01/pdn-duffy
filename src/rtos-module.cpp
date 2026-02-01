#include <rtos-module.hpp>

RtosModule::RtosModule()
    : _taskHandle(nullptr)
    , _queueHandle(nullptr)
    , _running(false)
{
}

RtosModule::~RtosModule()
{
    stop();
}

bool RtosModule::start()
{
    // Don't start if already running
    if (_running) {
        return false;
    }

    // Create the queue
    _queueHandle = xQueueCreate(getQueueLength(), getQueueItemSize());
    if (_queueHandle == nullptr) {
        return false;
    }

    // Create the task
    BaseType_t result = xTaskCreate(
        taskWrapper,        // Task function
        getTaskName(),      // Task name
        getStackSize(),     // Stack size
        this,               // Parameter (pointer to this instance)
        getPriority(),      // Priority
        &_taskHandle        // Task handle
    );

    if (result != pdPASS) {
        // Failed to create task, clean up queue
        vQueueDelete(_queueHandle);
        _queueHandle = nullptr;
        return false;
    }

    _running = true;
    return true;
}

void RtosModule::stop()
{
    if (!_running) {
        return;
    }

    _running = false;

    // Delete the task if it exists
    if (_taskHandle != nullptr) {
        vTaskDelete(_taskHandle);
        _taskHandle = nullptr;
    }

    // Delete the queue if it exists
    if (_queueHandle != nullptr) {
        vQueueDelete(_queueHandle);
        _queueHandle = nullptr;
    }
}

bool RtosModule::isRunning() const
{
    return _running;
}

bool RtosModule::sendToQueue(const void* data, TickType_t ticksToWait)
{
    if (_queueHandle == nullptr) {
        return false;
    }
    return xQueueSend(_queueHandle, data, ticksToWait) == pdTRUE;
}

bool RtosModule::sendToQueueFromISR(const void* data, BaseType_t* pxHigherPriorityTaskWoken)
{
    if (_queueHandle == nullptr) {
        return false;
    }
    return xQueueSendFromISR(_queueHandle, data, pxHigherPriorityTaskWoken) == pdTRUE;
}

bool RtosModule::receiveFromQueue(void* buffer, TickType_t ticksToWait)
{
    if (_queueHandle == nullptr) {
        return false;
    }
    return xQueueReceive(_queueHandle, buffer, ticksToWait) == pdTRUE;
}

UBaseType_t RtosModule::getQueueCount() const
{
    if (_queueHandle == nullptr) {
        return 0;
    }
    return uxQueueMessagesWaiting(_queueHandle);
}

void RtosModule::taskWrapper(void* pvParameters)
{
    RtosModule* module = static_cast<RtosModule*>(pvParameters);
    if (module != nullptr) {
        module->taskFunction();
    }
    
    // If taskFunction returns, delete the task
    module->_running = false;
    vTaskDelete(nullptr);
}
