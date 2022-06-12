#ifndef MODEM_TASKS_H
#define MODEM_TASKS_H
#include "modem.h"

void modem_task(void *pvParameter);

void advertising_task(void *pvParameter);

void test_emitter_task(void *pvParameter);

#endif