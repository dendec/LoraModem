#include "modem_tasks.h"
#include "message.h"
#include "tasks_arguments.h"

void modem_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    uint8_t buffer[SX127X_MAX_PACKET_LENGTH];
    extern SemaphoreHandle_t modem_semaphore;
    extern SemaphoreHandle_t txrx_semaphore;
    extern QueueHandle_t queue;
    while(true) {
        xSemaphoreTake( modem_semaphore, portMAX_DELAY );
        if (modem->state->receiving) {
            int16_t result = modem->radio->readData(buffer, SX127X_MAX_PACKET_LENGTH);
            if (result == ERR_NONE) {
                size_t len = modem->radio->getPacketLength();
                Packet packet = {};
                memcpy(&packet, buffer, len);
                if (packet.dst == modem->persister->getConfig()->address || packet.dst == BROADCAST_ADDR) {
                    uint8_t payload_len = 0;
                    if (modem->isImplicitHeader()) {
                        payload_len = PAYLOAD_SIZE;
                        while (payload_len > 0 && packet.payload[payload_len - 1] == 0) {
                            payload_len --;
                        }
                    } else {
                        payload_len = len - SERVICE_SIZE;
                    }
                    modem->state->nodes.addNode(packet.src, modem->radio->getRSSI());
                    if (payload_len > 0) {
                        ESP_LOGD(TAG, "Received %d bytes", payload_len);
                        ESP_LOGV(TAG, "Received %s", (char*)packet.payload);
                        modem->state->network.receive += payload_len;
                        modem->state->last_receive_time = millis();
                        Message message;
                        message.client_id = NULL;
                        message.len = payload_len;
                        message.to_transmit = false;
                        memcpy(message.data, packet.payload, payload_len);
                        xQueueSend(queue, (void *) &message, portMAX_DELAY);
                    } else {
                        ESP_LOGD(TAG, "Received service packet from %04X", packet.src);
                    }
                }
            } else {
                ESP_LOGE(TAG, "Receive failed. code: %d", result);
            }
        } else {
            ESP_LOGD(TAG, "Transmitted");
        }
        modem->receive();
        xSemaphoreGive(txrx_semaphore);
    }
}

void advertising_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    extern QueueHandle_t queue;
    while(true) {
        if (
            modem->persister->getConfig()->advertising_ms > 0 &&
            modem->state->receiving && 
            millis() - modem->state->last_receive_time > 1000 && 
            uxQueueMessagesWaiting(queue) == 0
        ) {
            modem->transmitAdvertisingPacket();
        }
        if (modem->persister->getConfig()->advertising_ms > 0) {
            vTaskDelay(modem->persister->getConfig()->advertising_ms / portTICK_RATE_MS);
        } else {
            vTaskDelay(DEFAULT_ADV_PERIOD_MS / portTICK_RATE_MS);
        }
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