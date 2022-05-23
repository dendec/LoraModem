#define DEBUGLOG_DISABLE_LOG
#include <DebugLog.h>
#include <SPI.h>
#include "config.h"
#include "modem.h"
#include "tasks/tasks.h"

SPIClass spi(HSPI);
Modem* modem;

void setupSerial() {
    Serial.begin(SERIAL_SPEED);
    Serial.setRxBufferSize(SERIAL_SIZE_RX);
    Serial.setTimeout(SERIAL_TIMEOUT_MS);
}

void setupModem() {
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    Module* mod = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_IO1, spi);
    SX1278* radio = new SX1278(mod);
    modem = new Modem(radio);
    modem->setup();
}

void setupTasks() {
    xQueueHandle messageQueue = xQueueCreate(5, sizeof(Message));
    static ModemWithQueueArg modemWithQueue = { .modem = modem, .queue = messageQueue };
    xTaskCreatePinnedToCore(&message_serial_reader_task, "serial", 2048, messageQueue, 5, NULL, 0);
    xTaskCreatePinnedToCore(&message_handler_task, "m_handler", 4096, &modemWithQueue, 5, NULL, 0);
    xTaskCreatePinnedToCore(&blink_task, "blink", 512, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(&receive_task, "rx", 4096, &modemWithQueue, 5, NULL, 1);
    xTaskCreatePinnedToCore(&transmit_task, "tx", 4096, modem, 5, NULL, 1);
    xTaskCreatePinnedToCore(&send_advertisement_task, "adv", 2048, modem, 5, NULL, 1);
    xTaskCreatePinnedToCore(&cleanup_routes_task, "cleanup", 2048, modem, 5, NULL, 1);
    //xTaskCreatePinnedToCore(&show_free_heap_task, "mem", 1024, NULL, 5, NULL, 1);
}

void setup() {
    setupSerial();
    setupModem();
    setupTasks();
}

void loop() {}