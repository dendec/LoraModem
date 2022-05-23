#include "display.h"

#ifdef HAS_OLED
const uint8_t ICON_ANT[] PROGMEM = { 0x49, 0x2a, 0x1c, 0x08, 0x08, 0x08, 0x08 }; //7x7

const uint8_t ICON_SIG[] PROGMEM = { 0x40, 0x40, 0x50, 0x50, 0x54, 0x54, 0x55 }; //7x7

const uint8_t ICON_UP[] PROGMEM = { 0x04, 0x0e, 0x15, 0x04, 0x04, 0x04, 0x04 }; //5x7

const uint8_t ICON_DOWN[] PROGMEM = { 0x04, 0x04, 0x04, 0x04, 0x15, 0x0e, 0x04 }; //5x7

const uint8_t ICON_USER[] PROGMEM = { 0xe0, 0xf0, 0x30, 0x30, 0x7d, 0x3f, 0x1e, 0x14 }; //8x8


ModemDisplay::ModemDisplay(uint8_t sda, uint8_t scl, uint8_t rst, uint8_t w, uint8_t h) { 
    display = new SSD1306Wire(0x3c, sda, scl);
    reset_pin = rst;
    width = w;
    height = h;
}
#endif

void ModemDisplay::setup() {
    #ifdef HAS_OLED
    ESP_LOGD(TAG, "Init display");
    pinMode(reset_pin, OUTPUT);
    digitalWrite(reset_pin, LOW);
    delay(50);
    digitalWrite(reset_pin, HIGH);
    if (display->init())
    {
        display->flipScreenVertically();
        display->clear();
        display->setColor(WHITE);
        display->setFont(ArialMT_Plain_10);
        ESP_LOGD(TAG, "Display initialised");
    } else {
        ESP_LOGE(TAG, "Failed to init display");
    }
    #endif
}

void ModemDisplay::apInfo(const char* ssid) {
    char message[38];
    sprintf(message, "ssid: %s", ssid);
    #ifdef HAS_OLED
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, message);
    showNetworkStat(0, 0);
    showClients(0);
    display->display();
    #else
    Serial.println(message);
    #endif
}

void ModemDisplay::staInfo(String ip, int8_t rssi) {
    char message[20];
    sprintf(message, "IP: %s", ip.c_str());
    #ifdef HAS_OLED
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, message);
    showWifiLevel(rssi);
    showNetworkStat(0, 0);
    showClients(0);
    display->display();
    #else
    Serial.println(message);
    #endif
}

void ModemDisplay::showWifiLevel(int8_t rssi) {
    #ifdef HAS_OLED
    display->drawXbm(width - 14, 3, 7, 7, ICON_ANT);
    updateWifiLevel(rssi);
    #endif
}

void ModemDisplay::updateWifiLevel(int8_t rssi) {
    #ifdef HAS_OLED
    ESP_LOGD(TAG, "WiFi rssi: %d", rssi);
    display->drawXbm(width - 7, 3, 7, 7, ICON_SIG);
    if (rssi > -80 && rssi <= -60) {
        clear(width - 2, 3, 2, 7);
    }
    if (rssi > -100 && rssi < -80) {
        clear(width - 4, 3, 4, 7);
    }
    if (rssi < -100) {
        clear(width - 6, 3, 6, 7);
    }
    display->display();
    #endif
}

void convertBytes(char* buffer, uint32_t bytes) {
    uint32_t value = bytes;
    uint8_t order = 0;
    while (value >= 1024) {
        value /= 1024;
        order++;
    }
    sprintf(buffer, "%d", value);
    switch (order)
    {
        case 1:
            strcat(buffer, "k"); break;
        case 2:
            strcat(buffer, "M"); break;
        case 3:
            strcat(buffer, "G"); break;
    }
}

void ModemDisplay::showNetworkStat(uint32_t tx, int32_t rx) {
    #ifdef HAS_OLED
    clear(0, 12, width, 7);
    display->drawXbm(0, 12, 5, 7, ICON_UP);
    display->drawXbm(48, 12, 5, 7, ICON_DOWN);
    updateNetworkStat(tx, rx);
    #endif
}

void ModemDisplay::updateNetworkStat(uint32_t tx, int32_t rx) {
    #ifdef HAS_OLED
    char buffer[6];
    convertBytes(buffer, tx);
    ESP_LOGV(TAG, "tx: %u", tx);
    clear(6, 12, 42, 7);
    display->drawString(6, 9, buffer);
    display->display();
    convertBytes(buffer, rx);
    ESP_LOGV(TAG, "rx: %u", rx);
    clear(54, 12, 42, 7);
    display->drawString(54, 9, buffer);
    display->display();
    #endif
}

void ModemDisplay::showClients(uint8_t clients) {
    #ifdef HAS_OLED
    display->drawXbm(width - 16, 12, 8, 8, ICON_USER);
    updateClients(clients);
    #endif
}

void ModemDisplay::updateClients(uint8_t clients) {
    #ifdef HAS_OLED
    clear(width - 8, 12, 11, 7);
    char buffer[2];
    if (clients > 9) {
        buffer[0] = '?';
        buffer[1] = '\0';
    } else {
        sprintf(buffer, "%1d", clients);
    }
    display->drawString(width - 7, 9, buffer);
    display->display();
    #endif
}

void ModemDisplay::updateRoutes(std::vector<RoutingTableRecord*> routes) {
    #ifdef HAS_OLED
    clear(0, 20, width, height);
    if (routes.size() > 0) {
        std::vector<RoutingTableRecord*>::iterator it;
        char buffer[32];
        unsigned long now = millis();
        uint8_t i = 0;
        for (it = routes.begin(); it != routes.end(); it++) {
            int32_t age = now - (*it)->received_millis;
            sprintf(buffer, "%04X %.1f", (*it)->address, (*it)->rssi);
            if ((*it)->adv_period_millis < age) {
                strcat(buffer, " ?");
            }
            display->drawString(0, 20 + 10*i, buffer);
        }
    }
    display->display();
    #endif
}

void ModemDisplay::message(const char* string) {
    #ifdef HAS_OLED
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display->drawString(width / 2, height / 2, string);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->display();
    #else
    Serial.println(string);
    #endif
}

void ModemDisplay::progress(uint8_t percent) {
    #ifdef HAS_OLED
    display->drawProgressBar(0, height - 9, width-1, 8, percent);
    display->display();
    #endif
}

void ModemDisplay::clear(int16_t x, int16_t y, int16_t w, int16_t h) {
    #ifdef HAS_OLED
    display->setColor(BLACK);
    display->fillRect(x, y, w, h);
    display->setColor(WHITE);
    display->display();
    #endif
}