#include <DebugLog.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "commands.h"
#include "message.h"
#include "message_tasks.h"

void message_serial_reader(void *pvParameter)
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

void send_to_clients(uint32_t* client_id, uint8_t* buffer, size_t size) {
    if (client_id != NULL) {
        //todo
    } else {
        Serial.write(buffer, size);
    }
}

void execute_or_transmit(CommandExecutor* executor, Modem* modem, Message* message) {
    LOG_INFO((char*)message->data);
    LOG_INFO(message->len);
    char buffer[256];
    switch (executor->execute((char*)message->data, message->len, buffer))
    {
        case NOT_EXECUTED:
            modem->transmit(message->data, message->len);
            break;
        case EXECUTED_UPDATED:
            modem->persister->saveConfig();
            modem->reset();
        case EXECUTED:
            send_to_clients(message->client_id, (uint8_t*) buffer, strlen(buffer));
    }
}

void message_handler(void *pvParameter)
{
    volatile ModemWithQueueArg* argument = (ModemWithQueueArg*) pvParameter;
    CommandExecutor* executor = new CommandExecutor(argument->modem->state, argument->modem->persister);
	while(true)
	{
        Message message;
        if (xQueueReceive(argument->queue, (void *) &message, 100) == pdTRUE) {
            if (message.to_transmit) {
                execute_or_transmit(executor, argument->modem, &message);
            } else {
                send_to_clients(message.client_id, message.data, message.len);
            }
        }
	    vTaskDelay(100 / portTICK_RATE_MS);
	}
    delete executor;
    vTaskDelete( NULL );
}