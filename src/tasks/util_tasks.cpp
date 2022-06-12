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
    // todo add semaphore
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    ModemDisplay* display = argument->display;
    extern SemaphoreHandle_t txrx_semaphore;
    while(display) {
        xSemaphoreTake( txrx_semaphore, portMAX_DELAY );
        Network* network = &modem->state->network;
        display->updateNetworkStat(network->transmit, network->receive);
    }
    vTaskDelete( NULL );
}

/**
 * This utility task runs only if modem in receive mode, 
 * nothing is going to be transmitted 
 * and last data packet was received quite long time ago. 
 * It does following actions:
 * - update number of connected web-clients on display;
 * - update Wi-Fi signal level on display;
 * - update Wi-Fi signal level in modem network state;
 * - clean up list of discovered nodes;
 * - update list of discovered nodes on display.
 */
void service_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    ModemDisplay* display = argument->display;
    ModemServer* server = argument->server;
    while(true) {
        uint32_t clients = 0;
        if (server->isStarted()) {
            clients = server->clients();
        }
        int8_t rssi = WiFi.RSSI();
        modem->state->network.rssi = rssi;
        modem->state->nodes.cleanUp();
        #ifdef HAS_OLED
        display->updateWifiLevel(rssi);
        display->updateClients(clients);
        display->updateNodes(modem->state->nodes.getNodes());
        #endif
        vTaskDelay(500 / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}

void display_task(void *pvParameter) {
    ModemDisplay* display = (ModemDisplay*) pvParameter;
    portTickType lastWakeTime = xTaskGetTickCount();
    while(true) {
        display->update();
        vTaskDelayUntil( &lastWakeTime, ( 50 / portTICK_RATE_MS ) ); // 20 FPS
    }
    vTaskDelete( NULL );
}

void test_emitter_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    extern SemaphoreHandle_t modem_semaphore;
    uint8_t cr = 0;
    uint8_t sf = 0;
    modem->transmitAdvertisingPacket();
    ModemConfig* config = modem->persister->getConfig();
    while(true) {
        xSemaphoreTake( modem_semaphore, portMAX_DELAY );
        config->radio.coding_rate = cr++ % 4 + 5;
        if (cr % 4 == 0) {
            config->radio.sfactor = sf++ % 7 + 6;
        }
        modem->setup();
        ESP_LOGI(TAG, "cr: %u, sf %u", config->radio.coding_rate, config->radio.sfactor );
        modem->transmitAdvertisingPacket();
    }
    vTaskDelete( NULL );
}