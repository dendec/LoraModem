#include <esp32-hal-gpio.h>
#include "button.h"
#include "FunctionalInterrupt.h"

QueueHandle_t queue_button = xQueueCreate(1, sizeof(int));

void IRAM_ATTR button_ISR() {
    int level = digitalRead(GPIO_NUM_0);
    xQueueSendToBackFromISR(queue_button, &level, nullptr);
}

Button::Button() {}

Button::~Button() {
    vTaskDelete(taskHandle);
    detachInterrupt(GPIO_NUM_0);
}

void Button::begin() {
    pinMode(GPIO_NUM_0, INPUT_PULLUP);
    attachInterrupt(GPIO_NUM_0, button_ISR, CHANGE);
    xTaskCreate(runTask, "gpioISRTask", 2048, this, 5, &taskHandle);
}

void Button::onPressed(OnButtonActionPerformed callback) {
    pressedCallback = callback;
}

void Button::onReleased(OnButtonActionPerformed callback) {
    releasedCallback = callback;
}

void Button::onLongPressed(OnButtonActionPerformed callback) {
    longPressedCallback = callback;
}

uint32_t Button::millis() {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void Button::runTask(void* arg) {
    Button* mui = (Button*)arg;

    int readLevel;
    uint32_t lastPressed = 0;

    while (true) {
        xQueueReceive(queue_button, &readLevel, portMAX_DELAY);
        if (readLevel == mui->RELEASED_STATE) {
            if (lastPressed > 0 && (mui->millis() - lastPressed > 3000)) {
                if (mui->longPressedCallback != nullptr) {
                    mui->longPressedCallback();
                }
            } else {
                if (mui->releasedCallback != nullptr) {
                    mui->releasedCallback();
                }
            }
        }

        if (readLevel == mui->PRESSED_STATE) {
            lastPressed = mui->millis();
            
            if (mui->pressedCallback != nullptr) {
                mui->pressedCallback();
            }
        }
    }
}