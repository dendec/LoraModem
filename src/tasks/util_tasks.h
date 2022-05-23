#ifndef UTIL_TASKS_H
#define UTIL_TASKS_H

void blink_task(void *pvParameter);

void show_free_heap_task(void *pvParameter);

void update_display_network_task(void *pvParameter);

void update_display_routes_task(void* pvParameter);

#endif