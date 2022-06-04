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
    for (auto it = nodes_table.begin(); it != nodes_table.end(); ++it) {
        nodes.push_back(&it->second);
    }
    return nodes;
}

void Nodes::cleanUp() {
    unsigned long now = millis();
    for (auto it = nodes_table.begin(); it != nodes_table.end(); ++it) {
        int32_t age = now - it->second.time;
        if (age > DEFAULT_ADV_PERIOD_MS * CLEAN_UP_THRESHOLD) {
            nodes_table.erase(it);
        }
    }
}