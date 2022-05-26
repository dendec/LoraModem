#include <esp32-hal-gpio.h>
#include <Esp.h>
#include "config.h"
#include "tasks_arguments.h"
#include "util_tasks.h"

void blink_task(void *pvParameter) {
    pinMode(BLINK_GPIO, OUTPUT);
    while(1) {
        digitalWrite(BLINK_GPIO, 0);
        vTaskDelay(100 / portTICK_RATE_MS);
        digitalWrite(BLINK_GPIO, 1);
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}

void show_free_heap_task(void *pvParameter) {
    while(1) {
        ESP_LOGD(TAG, "Free heap: %u", ESP.getFreeHeap());
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}

void update_display_network_task(void* pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    ModemDisplay* display = argument->display;
    while(1) {
        Network* network = &modem->state->network;
        display->updateNetworkStat(network->transmit, network->receive);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}

void update_display_routes_task(void* pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    ModemDisplay* display = argument->display;
    while(1) {
        display->updateNodes(modem->state->nodes.getNodes());
        vTaskDelay(ADVERTISING_PERIOD_MS / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}