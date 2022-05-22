#include <SPI.h>
#include "config.h"
#include "modem.h"
#include "tasks/tasks.h"

Modem* modem;

void setupSerial() {
    Serial.begin(SERIAL_SPEED);
    Serial.setRxBufferSize(SERIAL_SIZE_RX);
    Serial.setTimeout(SERIAL_TIMEOUT_MS);
}

void setupModem() {
    SPIClass spi(HSPI);
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    Module* mod = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_IO1, spi);
    SX1278* radio = new SX1278(mod);
    modem = new Modem(radio, [](uint8_t* data, size_t len){
        Serial.write(data, len);
    });
    modem->setup();
}

void setupTasks() {
    xQueueHandle messageQueue = xQueueCreate(5, sizeof(Message));
    xTaskCreatePinnedToCore(&message_serial_reader, "serial", 2048, messageQueue, 5, NULL, 0);
    static MessageHandlerArg messageHandlerArg = { .modem = modem, .queue = messageQueue };
    xTaskCreatePinnedToCore(&message_handler, "m_handler", 4096, &messageHandlerArg, 5, NULL, 0);
    xTaskCreatePinnedToCore(&blink_task, "blink", 512, NULL, 5, NULL, 1);
}

void setup() {
    setupSerial();
    setupModem();
    setupTasks();
}

void loop() {}