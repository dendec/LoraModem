#ifndef DISPLAY_H
#define DISPLAY_H
#include "SSD1306Wire.h"
#include "state.h"

class ModemDisplay {
    public:
        #ifdef HAS_OLED
        ModemDisplay(uint8_t sda, uint8_t scl, uint8_t rst, uint8_t w, uint8_t h);
        #endif
        void setup();
        void staInfo(String ip, int8_t rssi);
        void apInfo(const char* ssid);
        void message(const char* string);
        void progress(uint8_t percent);
        void showNetworkStat(uint32_t tx, int32_t rx);
        void updateNetworkStat(uint32_t tx, int32_t rx);
        void updateWifiLevel(int8_t rssi);
        void updateClients(uint8_t clients);
        void updateNodes(std::vector<Node*> nodes);
        void showMode(uint8_t mode);
        void update();
    private:
        void clear(int16_t x, int16_t y, int16_t w, int16_t h);
        void showWifiLevel(int8_t rssi);
        void showClients(uint8_t clients);
        SSD1306Wire* display;
        uint8_t reset_pin;
        uint8_t width;
        uint8_t height;
        bool is_updated = false;
};

#endif