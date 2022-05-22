#ifndef CONFIG_H
#define CONFIG_H

const uint8_t SSID_SIZE = 32;
const uint8_t PASSWORD_SIZE = 32;

enum WiFiMode { OFF = 0, AP, STA };

struct RadioConfig {
    float frequency;
    float bandwidth;
    uint8_t sfactor;
    uint8_t coding_rate;
    int8_t power;
    uint16_t preamble;
    uint8_t gain;
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
    uint8_t max_attempts = 0;
    uint16_t address = 0;
    uint16_t adv_period_millis = 10000;
};

class ConfigPersister {
    public:
        ConfigPersister();
        ModemConfig* getConfig();
        void saveConfig();
        void reset();
    private:
        ModemConfig* config = NULL;
        bool hasConfig = false;
};

#endif