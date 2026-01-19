#ifndef PDN_RTOS_HPP
#define PDN_RTOS_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>

// Global ring buffer handle for inter-task communication
extern RingbufHandle_t buf_handle;

#endif // PDN_RTOS_HPP
