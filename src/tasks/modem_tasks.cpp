#define DEBUGLOG_DISABLE_LOG
#include <DebugLog.h>
#include "modem_tasks.h"
#include "message.h"

void receive_task(void *pvParameter) {
    volatile ModemWithQueueArg* argument = (ModemWithQueueArg*) pvParameter;
    Modem* modem = argument->modem;
    xQueueHandle messageQueue = argument->queue;
    uint8_t buffer[SX127X_MAX_PACKET_LENGTH];
    while(true) {
        LOG_INFO("receive", modem->state->receive);
        LOG_INFO("is_received", modem->state->is_received);
        if (modem->state->receive && modem->state->is_received) {
            int64_t receive_time = millis();
            int16_t result = modem->radio->readData(buffer, SX127X_MAX_PACKET_LENGTH);
            if (result == ERR_NONE) {
                size_t len = modem->radio->getPacketLength();
                if (!modem->receiveAdvertisementPacket(buffer, len)) {
                    LOG_INFO(F("Received"), len, F("bytes in"), (int)(receive_time - modem->state->last_receive_time), F("ms"));
                    LOG_INFO((char*)buffer);
                    Packet packet = {};
                    memcpy(&packet, buffer, len);
                    if (packet.dst == modem->persister->getConfig()->address || packet.dst == BROADCAST_ADDR) {
                        uint8_t payload_len = len - SERVICE_SIZE;
                        //memcpy(modem->state->output + modem->state->output_len, packet.payload, payload_len);
                        //modem->state->output_len += payload_len;
                        modem->state->network.receive += payload_len;
                        modem->state->last_receive_time = receive_time;
                        Message message;
                        message.client_id = NULL;
                        message.len = payload_len;
                        message.to_transmit = false;
                        memcpy(message.data, packet.payload, payload_len);
                        xQueueSend(messageQueue, (void *) &message, 100);
                        LOG_INFO(F("Processed in"), (int)(millis() - receive_time),  F("ms"));
                    }
                }
            } else {
                LOG_ERROR(F("Receive failed. code:"), result);
            }
            modem->state->is_received = false;
        }
        vTaskDelay(1 / portTICK_RATE_MS);
    }
}

void transmit_task(void *pvParameter) {
    Modem* modem = (Modem*) pvParameter;
    while(true) {
        LOG_INFO("is_transmitted", modem->state->is_transmitted);
        if (!modem->state->receive && modem->state->is_transmitted) {
            LOG_INFO(F("Transmitted"));
            modem->state->is_transmitted = false;
            modem->startReceive();
        }
        vTaskDelay(1 / portTICK_RATE_MS);
    }
}

void send_advertisement_task(void *pvParameter) {
    Modem* modem = (Modem*) pvParameter;
    while(true) {
        if (!modem->state->is_received) {
            modem->transmitAdvertisementPacket();
        }
        vTaskDelay(modem->persister->getConfig()->adv_period_millis / portTICK_RATE_MS);
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