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
                if (!modem->receiveAdvertisementPacket(buffer, len)) {
                    ESP_LOGD(TAG, "Received %d bytes", len);
                    Packet packet = {};
                    memcpy(&packet, buffer, len);
                    if (packet.dst == modem->persister->getConfig()->address || packet.dst == BROADCAST_ADDR) {
                        uint8_t payload_len = len - SERVICE_SIZE;
                        modem->state->network.receive += payload_len;
                        modem->state->last_receive_time = millis();
                        Message message;
                        message.client_id = NULL;
                        message.len = payload_len;
                        message.to_transmit = false;
                        memcpy(message.data, packet.payload, payload_len);
                        xQueueSend(queue, (void *) &message, 100);
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

void send_advertisement_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    while(true) {
        uint16_t period = modem->persister->getConfig()->adv_period_millis;
        if (
            modem->state->receiving && 
            millis() - modem->state->last_receive_time > period && 
            uxQueueMessagesWaiting(queue) == 0
        ) {
            modem->transmitAdvertisementPacket();
        }
        vTaskDelay(period / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}

void cleanup_routes_task(void *pvParameter) {
    Modem* modem = (Modem*) pvParameter;
    while(true) {
        modem->state->routing_table.cleanUpRoutes();
        vTaskDelay(modem->persister->getConfig()->adv_period_millis / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
}