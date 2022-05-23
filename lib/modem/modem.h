#ifndef MODEM_H
#define MODEM_H
#include <RadioLib.h>
#include "config.h"
#include "state.h"

const uint8_t ADV_PREFIX_SIZE = 13;
const size_t SERVICE_SIZE = sizeof(uint16_t) * 2;
const size_t PAYLOAD_SIZE = PACKET_SIZE - SERVICE_SIZE - 1;

#pragma pack (1)
struct Packet {
    uint16_t src;
    uint16_t dst;
    uint8_t payload[PAYLOAD_SIZE];
};
#pragma pack(0)

struct AdvertisementPacket {
    char prefix[ADV_PREFIX_SIZE];
    uint16_t address;
    uint16_t adv_period_millis;
};

class Modem {

    public:
        Modem(SX1278* r, void (*send)(uint8_t* data, size_t len));
        void setup();
        void loop();
        void serviceLoop();
        void transmitPacket(uint8_t* data, size_t len);
        void transmitAdvertisementPacket();
        bool receiveAdvertisementPacket(uint8_t* buffer, size_t len);
        void reset();
        void startReceive();
        ModemConfig* config();
        ModemState* state;
        ConfigPersister* persister;
        SX1278* radio;
    private:
        void (*send)(uint8_t* data, size_t len);
        void error(const __FlashStringHelper* msg, int16_t code);
        void receive();
        void transmit(uint8_t* packet, size_t len);
        void flushOutput();
        void stopReceive();
};


#endif