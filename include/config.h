#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#define SERIAL_SIZE_RX 16384
#define SERIAL_SPEED 115200
#define SERIAL_TIMEOUT_MS 100

#ifdef HELTEC
#define LORA_IRQ 26
#define LORA_IO1 35
#define LORA_IO2 34
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_RST 14
#define LORA_CS 18
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define BLINK_GPIO 25
#endif

#ifdef CUSTOM
#define LORA_IRQ 2
#define LORA_IO1 4
#define LORA_SCK 14//5//14
#define LORA_MISO 12//19//12
#define LORA_MOSI 13//27//13
#define LORA_RST (uint8_t)0
#define LORA_CS 15
#endif
