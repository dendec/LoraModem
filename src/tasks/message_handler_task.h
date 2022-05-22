#ifndef MESSAGE_HANDLER_TASK_H
#define MESSAGE_HANDLER_TASK_H
#include "modem.h"

typedef struct {
    Modem* modem;
    xQueueHandle queue;
} MessageHandlerArg;

typedef struct {
    uint8_t data[PAYLOAD_SIZE];
    size_t len;
    uint32_t* client_id;
} Message;

void message_serial_reader(void *pvParameter);

void message_handler(void *pvParameter);

#endif