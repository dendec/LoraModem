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

QueueHandle_t queue;
SemaphoreHandle_t modem_semaphore;

void setupSerial() {
    Serial.begin(SERIAL_SPEED);
    Serial.setRxBufferSize(SERIAL_SIZE_RX);
    Serial.setTimeout(SERIAL_TIMEOUT_MS);
}

void setupDisplay() {
    #ifdef HAS_OLED
    display = new ModemDisplay(OLED_SDA, OLED_SCL, OLED_RST, OLED_WIDTH, OLED_HEIGHT);
    display->setup();
    #endif
}

void setupModem() {
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    Module* mod = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_IO1, spi);
    SX1278* radio = new SX1278(mod);
    modem = new Modem(radio);
    modem->setup();
}

void setupServer() {
    server = new ModemServer(modem, display, queue);
    server->setup();
}

void setupTasks() {
    static TaskArg taskArg = { 
        .modem = modem, 
        .display = display, 
        .server = server,
    };
    xTaskCreatePinnedToCore(&serial_reader_task, "serial", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(&message_handler_task, "m_handler", 4096, &taskArg, 2, NULL, 1);
    xTaskCreatePinnedToCore(&blink_task, "blink", 512, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(&modem_task, "modem", 4096, &taskArg, 3, NULL, 1);
    xTaskCreatePinnedToCore(&send_advertisement_task, "adv", 2048, &taskArg, 1, NULL, 1);
    xTaskCreatePinnedToCore(&cleanup_routes_task, "clean", 2048, modem, 1, NULL, 1);
    #ifdef HAS_OLED
    xTaskCreatePinnedToCore(&update_display_network_task, "d_net", 2048, &taskArg, 1, NULL, 1);
    xTaskCreatePinnedToCore(&update_display_routes_task, "d_route", 2048, &taskArg, 1, NULL, 1);
    #endif
    #if DCORE_DEBUG_LEVEL > 3
    xTaskCreatePinnedToCore(&show_free_heap_task, "mem", 2048, NULL, 1, NULL, 1);
    #endif
}

void setup() {
    queue = xQueueCreate(10, sizeof(Message));
    modem_semaphore = xSemaphoreCreateBinary();
    setupSerial();
    setupDisplay();
    setupModem();
    setupServer();
    setupTasks();
    ESP_LOGI(TAG, "Setup completed");
}

void loop() {}