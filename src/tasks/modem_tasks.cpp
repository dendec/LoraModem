#include "modem_tasks.h"
#include "message.h"
#include "tasks_arguments.h"

extern SemaphoreHandle_t modem_semaphore;
extern QueueHandle_t queue;

void modem_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    uint8_t buffer[SX127X_MAX_PACKET_LENGTH];
    while(true) {
        xSemaphoreTake( modem_semaphore, portMAX_DELAY );
        if (modem->state->receiving) {
            int16_t result = modem->radio->readData(buffer, SX127X_MAX_PACKET_LENGTH);
            if (result == ERR_NONE) {
                size_t len = modem->radio->getPacketLength();
                ESP_LOGD(TAG, "Received %d bytes", len);
                Packet packet = {};
                memcpy(&packet, buffer, len);
                if (packet.dst == modem->persister->getConfig()->address || packet.dst == BROADCAST_ADDR) {
                    uint8_t payload_len = len - SERVICE_SIZE;
                    if (payload_len == 0 && packet.dst == BROADCAST_ADDR) {
                        ESP_LOGI(TAG, "Received adv packet from %04X", packet.address);
                        modem->state->nodes.addNode(packet.src, modem->radio->getRSSI());
                    }
                    if (payload_len > 0) {
                        modem->state->network.receive += payload_len;
                        modem->state->last_receive_time = millis();
                        Message message;
                        message.client_id = NULL;
                        message.len = payload_len;
                        message.to_transmit = false;
                        memcpy(message.data, packet.payload, payload_len);
                        xQueueSend(queue, (void *) &message, 100 / portTICK_RATE_MS);
                    }
                }
            } else {
                ESP_LOGE(TAG, "Receive failed. code: %d", result);
            }
        } else {
            ESP_LOGD(TAG, "Transmitted");
        }
        modem->receive();
    }
}

void send_advertising_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    while(true) {
        if (
            modem->state->receiving && 
            millis() - modem->state->last_receive_time > ADVERTISING_PERIOD_MS && 
            uxQueueMessagesWaiting(queue) == 0
        ) {
            modem->transmitAdvertisingPacket();
        }
        vTaskDelay(ADVERTISING_PERIOD_MS / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}

void cleanup_routes_task(void *pvParameter) {
    Modem* modem = (Modem*) pvParameter;
    while(true) {
        modem->state->nodes.cleanUp();
        vTaskDelay(ADVERTISING_PERIOD_MS / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}