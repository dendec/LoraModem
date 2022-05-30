#ifndef CONFIG_H
#define CONFIG_H
#include "constants.h"

enum WiFiMode { OFF = 0, AP, STA };

struct RadioConfig {
    float frequency;
    float bandwidth;
    uint8_t sfactor;
    uint8_t coding_rate;
    int8_t power;
    bool crc;
};

struct NetworkConfig {
    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];
    WiFiMode WIFI_mode;         //0-disabled, 1-AP, 2-STA
    uint8_t channel;
};

struct ModemConfig {
    RadioConfig radio;
    NetworkConfig network;
    uint16_t address = 0;
};

class ConfigPersister {
    public:
        ConfigPersister();
        ModemConfig* getConfig();
        void saveConfig();
        void reset();
    private:
        ModemConfig* config = nullptr;
        bool hasConfig = false;
};

#endif