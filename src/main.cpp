#include <Arduino.h>
#include <DebugLog.h>
#include <SPI.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "modem.h"
#include "tasks/tasks.h"

SPIClass spi(HSPI);
Modem* modem;

/*typedef struct {
    uint8_t data[PAYLOAD_SIZE];
    size_t len;
    uint32_t* client_id;
} message_t;

void serial_read_task(void *pvParameter)
{
    xQueueHandle messageQueue = (xQueueHandle) pvParameter;
	while(true)
	{
        if (Serial.available() > 0) {
            message_t message;
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
    xQueueHandle messageQueue = (xQueueHandle) pvParameter;
    CommandExecutor* executor = new CommandExecutor(modem->state, modem->persister);
    char buffer[256];
	while(true)
	{
        message_t message;
        if (xQueueReceive(messageQueue, (void *) &message, 100) == pdTRUE) {
            switch (executor->execute((char*)message.data, message.len, buffer))
            {
                case NOT_EXECUTED:
                    modem->transmit(message.data, message.len);
                    break;
                case EXECUTED:
                    if (message.client_id == NULL) {
                        Serial.write(buffer);
                    }
                case EXECUTED_UPDATED:
                    modem->persister->saveConfig();
                    modem->reset();
                    break;
                default:
                    break;
            }
            //Serial.write(message.input, message.len);
        }
	    vTaskDelay(100 / portTICK_RATE_MS);
	}
    vTaskDelete( NULL );
    delete executor; 
}*/

void blinky(void *pvParameter)
{
    pinMode(BLINK_GPIO, OUTPUT);
    while(1) {
        digitalWrite(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        digitalWrite(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void setupModem() {
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    Module* mod = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_IO1, spi);
    SX1278* radio = new SX1278(mod);
    modem = new Modem(radio, [](uint8_t* data, size_t len){
        Serial.write(data, len);
    });
    modem->setup();
}

void setup() {
    Serial.begin(SERIAL_SPEED);
    Serial.setRxBufferSize(SERIAL_SIZE_RX);
    Serial.setTimeout(SERIAL_TIMEOUT_MS);
    setupModem();
    xQueueHandle messageQueue = xQueueCreate(5, sizeof(Message));
    xTaskCreatePinnedToCore(&message_serial_reader, "serial", 2048, messageQueue, 5, NULL, 0);
    static MessageHandlerArg messageHandlerArg = { .modem = modem, .queue = messageQueue };
    xTaskCreatePinnedToCore(&message_handler, "m_handler", 4096, &messageHandlerArg, 5, NULL, 0);
    xTaskCreatePinnedToCore(&blinky, "blinky", 512, NULL, 5, NULL, 1);
}

void loop() {}