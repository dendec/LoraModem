#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "commands.h"
#include "message.h"
#include "message_tasks.h"
#include "tasks_arguments.h"

void message_serial_reader_task(void *pvParameter)
{
    xQueueHandle messageQueue = (xQueueHandle) pvParameter;
	while(true)
	{
        if (Serial.available() > 0) {
            Message message;
            message.client_id = NULL;
            message.len = Serial.readBytes(message.data, PAYLOAD_SIZE);
            message.to_transmit = true;
            xQueueSend(messageQueue, (void *) &message, 100);
        }
	    vTaskDelay(100 / portTICK_RATE_MS);
	}
    vTaskDelete( NULL ); 
}

void send_to_clients(ModemServer* server, uint32_t* client_id, uint8_t* buffer, size_t size) {
    Serial.write(buffer, size);
    if (server != nullptr) {
        server->send(client_id, buffer, size);
    }
}

boolean execute_or_transmit(ModemServer* server, CommandExecutor* executor, Modem* modem, Message* message) {
    ESP_LOGV(TAG, "%s", (char*)message->data);
    ESP_LOGV(TAG, "%d", message->len);
    char buffer[256];
    switch (executor->execute((char*)message->data, message->len, buffer))
    {
        case NOT_EXECUTED:
            if (modem->state->receiving && !modem->state->is_transmitted) {
                modem->transmitPacket(message->data, message->len);
            } else {
                return false;
            }
            break;
        case EXECUTED_UPDATED:
            modem->persister->saveConfig();
            modem->reset();
        case EXECUTED:
            send_to_clients(server, message->client_id, (uint8_t*) buffer, strlen(buffer));
    }
    return true;
}

void message_handler_task(void *pvParameter)
{
    volatile TaskArg* argument = (TaskArg*) pvParameter;
    Modem* modem = argument->modem;
    ModemServer* server = argument->server;
    xQueueHandle queue = argument->queue;
    CommandExecutor* executor = new CommandExecutor(modem->state, modem->persister);
    while(true)
	{
        Message message;
        if (xQueuePeek(queue, (void *) &message, 100) == pdTRUE) {
            boolean is_transmitted = true;
            if (message.to_transmit) {
                is_transmitted &= execute_or_transmit(server, executor, modem, &message);
            } else {
                send_to_clients(server, message.client_id, message.data, message.len);
            }
            if(is_transmitted) {
                xQueueReceive(queue, (void *) &message, 100);
            }
        }
	    vTaskDelay(1 / portTICK_RATE_MS);
	}
    delete executor;
    vTaskDelete( NULL );
}