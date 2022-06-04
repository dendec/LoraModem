#include <SPI.h>
#include "button.h"
#include "config.h"
#include "commands.h"
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
SemaphoreHandle_t txrx_semaphore;

void setupSerial() {
    Serial.begin(SERIAL_SPEED);
    Serial.setRxBufferSize(SERIAL_SIZE_RX);
    Serial.setTimeout(SERIAL_TIMEOUT_MS);
}

void setupDisplay() {
    #ifdef HAS_OLED
    display = new ModemDisplay(OLED_SDA, OLED_SCL, OLED_RST, OLED_WIDTH, OLED_HEIGHT);
    display->setup();
    xTaskCreatePinnedToCore(&display_task, "display", 2048, display, 1, NULL, 1);
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
    xTaskCreatePinnedToCore(&message_handler_task, "handler", 4096, &taskArg, 2, NULL, 1);
    xTaskCreatePinnedToCore(&blink_task, "blink", 1024, NULL, 1, NULL, 1);
    #if TEST_EMITTER
    xTaskCreatePinnedToCore(&test_emitter_task, "emitter", 4096, &taskArg, 3, NULL, 1);
    #else
    xTaskCreatePinnedToCore(&modem_task, "modem", 4096, &taskArg, 3, NULL, 1);
    xTaskCreatePinnedToCore(&service_task, "service", 2048, &taskArg, 1, NULL, 1);
    xTaskCreatePinnedToCore(&advertising_task, "advertising", 2048, &taskArg, 1, NULL, 1);
    #endif
    xTaskCreatePinnedToCore(&update_display_network_task, "d_net", 2048, &taskArg, 1, NULL, 1);
    #if CORE_DEBUG_LEVEL > 3
    xTaskCreatePinnedToCore(&show_free_heap_task, "mem", 2048, NULL, 1, NULL, 1);
    #endif
}

void setupButton() {
    Button* button = new Button();
    CommandMode* commandMode = new CommandMode(modem->persister->getConfig());
    button->onReleased([=]() {
        char buffer[3];
        commandMode->executeQuery(buffer);
        commandMode->argument = (atoi(buffer) +1 ) % 28;
        commandMode->executeAssign(buffer);
        modem->persister->saveConfig();
        modem->reset();
        display->showMode(commandMode->argument);
        ESP_LOGD(TAG, "mode %d", commandMode->argument);
    });
    button->onLongPressed([=]() {
        modem->persister->reset();
        modem->reset();
        ESP.restart();
    });
    button->begin();
}

void setup() {
    queue = xQueueCreate(16, sizeof(Message));
    modem_semaphore = xSemaphoreCreateBinary();
    txrx_semaphore = xSemaphoreCreateBinary();
    setupSerial();
    setupDisplay();
    setupModem();
    setupServer();
    setupTasks();
    setupButton();
    ESP_LOGI(TAG, "Setup completed");
    vTaskDelete(NULL);
}

void loop() {}