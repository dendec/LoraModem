#include "modem_tasks.h"
#include "message.h"
#include "tasks_arguments.h"

void receive_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    xQueueHandle queue = argument->queue;
    uint8_t buffer[SX127X_MAX_PACKET_LENGTH];
    while(true) {
        if (modem->state->receiving && modem->state->is_received) {
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
            modem->state->is_received = false;
            modem->receive();
        }
        vTaskDelay(1 / portTICK_RATE_MS);
    }
}

void transmit_task(void *pvParameter) {
    Modem* modem = (Modem*) pvParameter;
    while(true) {
        if (!modem->state->receiving && modem->state->is_transmitted) {
            ESP_LOGD(TAG, "Transmitted");
            modem->state->is_transmitted = false;
            modem->receive();
        }
        vTaskDelay(1 / portTICK_RATE_MS);
    }
}

void send_advertisement_task(void *pvParameter) {
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    xQueueHandle queue = argument->queue;
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