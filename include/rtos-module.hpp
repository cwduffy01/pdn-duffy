#ifndef RTOS_MODULE_HPP
#define RTOS_MODULE_HPP

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/**
 * @brief Abstract base class for RTOS modules with a task and queue.
 * 
 * Derived classes must implement:
 * - taskFunction(): The main task loop logic
 * - getTaskName(): Returns the task name
 * - getStackSize(): Returns the stack size in bytes
 * - getPriority(): Returns the task priority
 * - getQueueLength(): Returns the queue length
 * - getQueueItemSize(): Returns the size of each queue item
 */
class RtosModule {
public:
    RtosModule();
    virtual ~RtosModule();

    /**
     * @brief Initialize and start the RTOS task and queue.
     * @return true if initialization was successful, false otherwise.
     */
    bool start(bool asSuspended = false);

    /**
     * @brief Stop and delete the RTOS task.
     */
    void stop();

    void suspend();

    void resume();

    void notify(uint32_t notif_value);

    /**
     * @brief Check if the task is currently running.
     * @return true if running, false otherwise.
     */
    bool isRunning() const;

    /**
     * @brief Send data to the module's queue.
     * @param data Pointer to the data to send.
     * @param ticksToWait Maximum time to wait for space in the queue.
     * @return true if data was sent successfully, false otherwise.
     */
    bool sendToQueue(const void* data, TickType_t ticksToWait = portMAX_DELAY);

    /**
     * @brief Send data to the module's queue from an ISR.
     * @param data Pointer to the data to send.
     * @param pxHigherPriorityTaskWoken Set to pdTRUE if a higher priority task was woken.
     * @return true if data was sent successfully, false otherwise.
     */
    bool sendToQueueFromISR(const void* data, BaseType_t* pxHigherPriorityTaskWoken);

protected:
    /**
     * @brief The main task function to be implemented by derived classes.
     * 
     * This function should contain the task's main loop. It will be called
     * once when the task starts and should contain its own loop if needed.
     */
    virtual void taskFunction() = 0;

    /**
     * @brief Get the task name.
     * @return The name of the task.
     */
    virtual const char* getTaskName() const = 0;

    /**
     * @brief Get the stack size for the task.
     * @return Stack size in bytes.
     */
    virtual uint32_t getStackSize() const = 0;

    /**
     * @brief Get the task priority.
     * @return Task priority (higher number = higher priority).
     */
    virtual UBaseType_t getPriority() const = 0;

    /**
     * @brief Get the queue length.
     * @return Number of items the queue can hold.
     */
    virtual UBaseType_t getQueueLength() const = 0;

    /**
     * @brief Get the size of each queue item.
     * @return Size in bytes of each queue item.
     */
    virtual UBaseType_t getQueueItemSize() const = 0;

    /**
     * @brief Receive data from the queue (for use within taskFunction).
     * @param buffer Pointer to buffer where received data will be stored.
     * @param ticksToWait Maximum time to wait for data.
     * @return true if data was received, false otherwise.
     */
    bool receiveFromQueue(void* buffer, TickType_t ticksToWait = portMAX_DELAY);

    /**
     * @brief Get the number of items currently in the queue.
     * @return Number of items waiting in the queue.
     */
    UBaseType_t getQueueCount() const;

    /// Handle to the FreeRTOS task
    TaskHandle_t _taskHandle;

    /// Handle to the FreeRTOS queue
    QueueHandle_t _queueHandle;

private:
    /**
     * @brief Static task wrapper that calls the virtual taskFunction.
     * @param pvParameters Pointer to the RtosModule instance.
     */
    static void taskWrapper(void* pvParameters);

    /// Flag indicating if the task is running
    volatile bool _running;
};

#endif // RTOS_MODULE_HPP
