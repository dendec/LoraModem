#include <Arduino.h>
#include <stdio.h>
#include "state.h"

const uint8_t CLEAN_UP_THRESHOLD = 5;

void RoutingTable::addRoute(uint16_t address, uint16_t adv_period_millis, float rssi) {
    routing_table[address] = RoutingTableRecord{address, adv_period_millis, rssi, millis()};
}

RoutingTableRecord* RoutingTable::getRoute(uint16_t address) {
    return &routing_table[address];
}

std::vector<RoutingTableRecord*> RoutingTable::getRoutes() {
    std::vector<RoutingTableRecord*> routes;
    std::map<uint16_t, RoutingTableRecord>::iterator it;
    for (it = routing_table.begin(); it != routing_table.end(); it++) {
        routes.push_back(&it->second);
    }
    return routes;
}

void RoutingTable::cleanUpRoutes() {
    std::map<uint16_t, RoutingTableRecord>::iterator it;
    unsigned long now = millis();
    for (it = routing_table.begin(); it != routing_table.end(); it++) {
        int32_t age = now - it->second.received_millis;
        if (age > it->second.adv_period_millis * CLEAN_UP_THRESHOLD) {
            routing_table.erase(it);
        }
    }
}