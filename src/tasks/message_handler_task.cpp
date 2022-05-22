#include <DebugLog.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "message_handler_task.h"

void message_serial_reader(void *pvParameter)
{
    xQueueHandle messageQueue = (xQueueHandle) pvParameter;
	while(true)
	{
        if (Serial.available() > 0) {
            Message message;
            message.client_id = NULL;
            message.len = Serial.readBytes(message.data, PAYLOAD_SIZE);
            xQueueSend(messageQueue, (void *) &message, 100);
        }
	    vTaskDelay(100 / portTICK_RATE_MS);
	}
    vTaskDelete( NULL ); 
}

void message_handler(void *pvParameter)
{
    volatile MessageHandlerArg* argument = (MessageHandlerArg*) pvParameter;
    CommandExecutor* executor = new CommandExecutor(argument->modem->state, argument->modem->persister);
    char buffer[256];
	while(true)
	{
        Message message;
        if (xQueueReceive(argument->queue, (void *) &message, 100) == pdTRUE) {
            switch (executor->execute((char*)message.data, message.len, buffer))
            {
                case NOT_EXECUTED:
                    argument->modem->transmit(message.data, message.len);
                    break;
                case EXECUTED:
                    if (message.client_id == NULL) {
                        Serial.write(buffer);
                    }
                case EXECUTED_UPDATED:
                    argument->modem->persister->saveConfig();
                    argument->modem->reset();
                    break;
                default:
                    break;
            }
        }
	    vTaskDelay(100 / portTICK_RATE_MS);
	}
    delete executor; 
    vTaskDelete( NULL );
}