#include <Arduino.h>
#include <stdio.h>
#include "state.h"

void Nodes::addNode(uint16_t address, float rssi) {
    nodes_table[address] = Node{address, rssi, millis()};
}

Node* Nodes::getNode(uint16_t address) {
    return &nodes_table[address];
}

std::vector<Node*> Nodes::getNodes() {
    std::vector<Node*> nodes;
    std::map<uint16_t, Node>::iterator it;
    for (it = nodes_table.begin(); it != nodes_table.end(); it++) {
        nodes.push_back(&it->second);
    }
    return nodes;
}

void Nodes::cleanUp() {
    std::map<uint16_t, Node>::iterator it;
    unsigned long now = millis();
    for (it = nodes_table.begin(); it != nodes_table.end(); it++) {
        int32_t age = now - it->second.time;
        if (age > ADVERTISING_PERIOD_MS * CLEAN_UP_THRESHOLD) {
            nodes_table.erase(it);
        }
    }
}