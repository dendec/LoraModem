//#define DEBUGLOG_DISABLE_LOG
#include <DebugLog.h>
#include "modem_tasks.h"
#include "message.h"

void receive_task(void *pvParameter) {
    volatile ModemWithQueueArg* argument = (ModemWithQueueArg*) pvParameter;
    while(true) {
        argument->modem->loop();
        vTaskDelay(1 / portTICK_RATE_MS);
    }
}

void send_advertisement_task(void *pvParameter) {
    Modem* modem = (Modem*) pvParameter;
    while(true) {
        modem->transmitAdvertisementPacket();
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