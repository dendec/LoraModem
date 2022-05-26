#ifndef MODEM_TASKS_H
#define MODEM_TASKS_H
#include "modem.h"

void modem_task(void *pvParameter);

void send_advertising_task(void *pvParameter);

void cleanup_routes_task(void *pvParameter);

#endif