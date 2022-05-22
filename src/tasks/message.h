#ifndef MESSAGE_H
#define MESSAGE_H
#include "modem.h"

typedef struct {
    Modem* modem;
    xQueueHandle queue;
} ModemWithQueueArg;

typedef struct {
    uint8_t data[PAYLOAD_SIZE];
    size_t len;
    uint32_t* client_id;
    boolean to_transmit;
} Message;

#endif