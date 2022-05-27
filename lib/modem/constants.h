#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <stddef.h>
#include <stdint.h>

const uint8_t SSID_SIZE = 32;
const uint8_t PASSWORD_SIZE = 32;

const uint16_t SERVICE_PERIOD_MS = 10000;
const uint8_t CLEAN_UP_THRESHOLD = 5;

const uint16_t BROADCAST_ADDR = 0xFFFF;
const size_t PACKET_SIZE = 255;
const size_t SERVICE_SIZE = sizeof(uint16_t) * 2;
const size_t PAYLOAD_SIZE = PACKET_SIZE - SERVICE_SIZE - 1;

#endif