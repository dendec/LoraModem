#ifndef STATE_H
#define STATE_H
#include <Stream.h>
#include <vector>
#include <map>
#include "constants.h"

struct Node {
    uint16_t address;
    float rssi;
    uint32_t time;
};

class Nodes {
    public:
        void addNode(uint16_t address, float rssi);
        Node* getNode(uint16_t address);
        std::vector<Node*> getNodes();
        void cleanUp();
    private:
        std::map<uint16_t, Node> nodes_table;
};

struct Network {
    uint32_t transmit;
    uint32_t receive;
    uint32_t ip;
    int8_t rssi;
};

struct ModemState {
    uint16_t address_destination = BROADCAST_ADDR;
    Nodes nodes;
    Network network;
    bool receiving = false;
    int64_t last_receive_time = 0;
};

#endif