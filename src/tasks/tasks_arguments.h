#ifndef TASKS_ARGUMENTS_H
#define TASKS_ARGUMENTS_H
#include "modem.h"
#include "display.h"

typedef struct {
    Modem* modem;
    ModemDisplay* display;
    xQueueHandle queue;
} TaskArg;

#endif