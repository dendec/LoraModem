#ifndef MODEM_H
#define MODEM_H
#include <RadioLib.h>
#include "config.h"
#include "state.h"

const size_t SERVICE_SIZE = sizeof(uint16_t) * 2;
const size_t PAYLOAD_SIZE = PACKET_SIZE - SERVICE_SIZE - 1;

#pragma pack (1)
struct Packet {
    uint16_t src;
    uint16_t dst;
    uint8_t payload[PAYLOAD_SIZE];
};
#pragma pack(0)

class Modem {

    public:
        Modem(SX1278* r);
        void setup();
        void transmitPacket(uint8_t* data, size_t len);
        void transmitAdvertisementPacket();
        bool receiveAdvertisementPacket(uint8_t* buffer, size_t len);
        void reset();
        void receive();
        ModemState* state;
        ConfigPersister* persister;
        SX1278* radio;
    private:
        void transmit(uint8_t* packet, size_t len);
};


#endif