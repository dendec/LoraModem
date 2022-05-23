#include <SPI.h>
#include "config.h"
#include "display.h"
#include "message.h"
#include "modem.h"
#include "server.h"
#include "tasks/tasks.h"

SPIClass spi(HSPI);
Modem* modem;
ModemDisplay* display;
ModemServer* server;

void setupSerial() {
    Serial.begin(SERIAL_SPEED);
    Serial.setRxBufferSize(SERIAL_SIZE_RX);
    Serial.setTimeout(SERIAL_TIMEOUT_MS);
}

void setupDisplay() {
    #ifdef HAS_OLED
    display = new ModemDisplay(OLED_SDA, OLED_SCL, OLED_RST, OLED_WIDTH, OLED_HEIGHT);
    display->setup();
    #else
    display = new ModemDisplay();
    #endif
}

void setupModem() {
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    Module* mod = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_IO1, spi);
    SX1278* radio = new SX1278(mod);
    modem = new Modem(radio);
    modem->setup();
}

void setupServer(xQueueHandle messageQueue) {
    server = new ModemServer(modem, display, messageQueue);
    server->setup();
}

void setupTasks(xQueueHandle messageQueue) {
    static TaskArg taskArg = { .modem = modem, .display = display, .server = server, .queue = messageQueue };
    xTaskCreatePinnedToCore(&message_serial_reader_task, "serial", 2048, messageQueue, 5, NULL, 0);
    xTaskCreatePinnedToCore(&message_handler_task, "m_handler", 4096, &taskArg, 5, NULL, 0);
    xTaskCreatePinnedToCore(&blink_task, "blink", 512, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(&receive_task, "rx", 4096, &taskArg, 10, NULL, 1);
    xTaskCreatePinnedToCore(&transmit_task, "tx", 4096, modem, 10, NULL, 1);
    xTaskCreatePinnedToCore(&send_advertisement_task, "adv", 2048, &taskArg, 1, NULL, 1);
    xTaskCreatePinnedToCore(&cleanup_routes_task, "clean", 2048, modem, 1, NULL, 1);
    xTaskCreatePinnedToCore(&update_display_network_task, "d_net", 2048, &taskArg, 1, NULL, 1);
    xTaskCreatePinnedToCore(&update_display_routes_task, "d_route", 2048, &taskArg, 1, NULL, 1);
    //xTaskCreatePinnedToCore(&show_free_heap_task, "mem", 2048, NULL, 1, NULL, 1);
}

void setup() {
    xQueueHandle messageQueue = xQueueCreate(10, sizeof(Message));
    setupSerial();
    setupDisplay();
    setupModem();
    setupServer(messageQueue);
    setupTasks(messageQueue);
    ESP_LOGI(TAG, "Setup completed");
}

void loop() {}