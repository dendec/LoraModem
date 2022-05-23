#ifndef TASKS_ARGUMENTS_H
#define TASKS_ARGUMENTS_H
#include "display.h"
#include "modem.h"
#include "server.h"

typedef struct {
    Modem* modem;
    ModemDisplay* display;
    ModemServer* server;
    xQueueHandle queue;
} TaskArg;

#endif