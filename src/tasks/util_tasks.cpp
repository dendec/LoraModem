#include <DebugLog.h>
#include <esp32-hal-gpio.h>
#include "config.h"
#include "util_tasks.h"

void blink_task(void *pvParameter)
{
    pinMode(BLINK_GPIO, OUTPUT);
    while(1) {
        digitalWrite(BLINK_GPIO, 0);
        vTaskDelay(100 / portTICK_RATE_MS);
        digitalWrite(BLINK_GPIO, 1);
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}

void show_free_heap_task(void *pvParameter)
{
    while(1) {
        LOG_INFO(F("Free heap:"), ESP.getFreeHeap());
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}
