#ifndef GPIO_INTERRUPT_SERVICE_H
#define GPIO_INTERRUPT_SERVICE_H

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <functional>

class Button {
public:
    typedef std::function<void(void)> OnButtonActionPerformed;

    Button();
    ~Button();
    void begin();
    void onPressed(OnButtonActionPerformed callback);
    void onReleased(OnButtonActionPerformed callback);
    void onLongPressed(OnButtonActionPerformed callback);

private:
    static void runTask(void* arg);
    uint32_t millis();

    OnButtonActionPerformed pressedCallback = nullptr;
    OnButtonActionPerformed releasedCallback = nullptr;
    OnButtonActionPerformed longPressedCallback = nullptr;
    const int PRESSED_STATE = 0;
    const int RELEASED_STATE = 1;
    TaskHandle_t taskHandle;
};

#endif