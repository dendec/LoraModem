#include <esp32-hal-gpio.h>
#include "config.h"
#include "blink_task.h"

void blink_task(void *pvParameter)
{
    pinMode(BLINK_GPIO, OUTPUT);
    while(1) {
        digitalWrite(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        digitalWrite(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
