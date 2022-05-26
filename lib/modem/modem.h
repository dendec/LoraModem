#ifndef MODEM_H
#define MODEM_H
#include <RadioLib.h>
#include "config.h"
#include "state.h"

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
        void transmitAdvertisingPacket();
        void reset();
        void receive();
        ModemState* state;
        ConfigPersister* persister;
        SX1278* radio;
    private:
        void transmit(uint8_t* packet, size_t len);
};


#endif