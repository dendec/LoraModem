#include <DebugLog.h>
#include <EEPROM.h>
#include <WiFi.h>
#include "config.h"

ConfigPersister::ConfigPersister() {
    EEPROM.begin(sizeof(ModemConfig) + 1);
}

ModemConfig* ConfigPersister::getConfig() {
    if (config == nullptr) {
        config = new ModemConfig();
    }
    if (hasConfig) {
        return config;
    }
    bool configPersisted = false;
    EEPROM.get(0, configPersisted);
    if (configPersisted) {
        EEPROM.get(1, *config);
        LOG_DEBUG(F("Config loaded"));
    } else {
        config->radio = RadioConfig{434.0, 500.0, 9, 7, 10, 8, 0};
        config->network = NetworkConfig{{}, {}, AP, 1};
        byte mac[6];
        WiFi.macAddress(mac);
        memcpy(&config->address, mac + 4, sizeof(uint16_t)); // last 2 bytes
        LOG_DEBUG(F("Default config loaded"));
    }
    hasConfig = true;
    return config;
}

void ConfigPersister::saveConfig() {
    EEPROM.write(0, true);
    EEPROM.put(1, *config);
    EEPROM.commit();
}

void ConfigPersister::reset() {
    EEPROM.write(0, false);
    EEPROM.commit();
    hasConfig = false;
}
