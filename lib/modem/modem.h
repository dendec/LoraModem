#ifndef MODEM_H
#define MODEM_H
#include <RadioLib.h>
#include "commands.h"
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
        void transmit(uint8_t* data, size_t len);
        void reset();
        ModemConfig* config();
        ModemState* state;
        ConfigPersister* persister;
    private:
        SX1278* radio;
        CommandExecutor* executor;
        void (*send)(uint8_t* data, size_t len);
        void error(const __FlashStringHelper* msg, int16_t code);
        void receive();
        bool receiveAdvertisementPacket(size_t len);
        void transmitAdvertisementPacket();
        void startReceive();
        void flushOutput();
        void stopReceive();
};


#endif