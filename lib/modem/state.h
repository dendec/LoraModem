#ifndef STATE_H
#define STATE_H
#include <Stream.h>
#include <map>

const uint16_t BROADCAST_ADDR = 0xFFFF;
const size_t PACKET_SIZE = 255;
const size_t BUFFER_SIZE = PACKET_SIZE * 10;

struct RoutingTableRecord {
    uint16_t address;
    uint16_t adv_period_millis;
    float rssi;
    uint32_t received_millis;
};

class RoutingTable {
    public:
        void addRoute(uint16_t address, uint16_t adv_period_millis, float rssi);
        RoutingTableRecord* getRoute(uint16_t address);
        std::vector<RoutingTableRecord*> getRoutes();
        void cleanUpRoutes();
    private:
        std::map<uint16_t, RoutingTableRecord> routing_table;
};

struct Network {
    uint32_t transmit;
    uint32_t receive;
    uint32_t ip;
    int8_t rssi;
};

struct ModemState {
    uint16_t address_destination = BROADCAST_ADDR;
    uint8_t buffer[PACKET_SIZE];
    size_t input_len = 0;
    uint8_t output[BUFFER_SIZE];
    size_t output_len = 0;
    RoutingTable routing_table;
    Network network;
    bool receive = false;
    bool is_received = false;
    int64_t last_receive_time = 0;
};

#endif