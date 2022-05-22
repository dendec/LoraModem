//#define DEBUGLOG_RELEASE_MODE
#include <DebugLog.h>
#include "config.h"
#include "modem.h"

const char* ADV_PREFIX = "ADVERTISEMENT";
const uint16_t RX_TIMEOUT_MS = 500;
const uint16_t TX_DELAY_MS = 25; //time for receiver to handle packet
extern Modem* modem;

Modem::Modem(SX1278* r, void (*send)(uint8_t* data, size_t len)): radio(r), send(send) {
    persister = new ConfigPersister();
    state = new ModemState();
    executor = new CommandExecutor(state, persister);
}

ModemConfig* Modem::config() {
    return persister->getConfig();
}

void Modem::setup() {
    int16_t err = radio->begin(
        config()->radio.frequency, 
        config()->radio.bandwidth, 
        config()->radio.sfactor, 
        config()->radio.coding_rate, 
        SX127X_SYNC_WORD, 
        config()->radio.power, 
        config()->radio.preamble,
        config()->radio.gain);
    if (err != ERR_NONE) {
        error(F("LoRa initialization"), err);
        persister->reset();
        setup();
    }
}

void Modem::loop() {
    if (state->receive) {
        receive();
        int64_t now = millis();
        if (now - state->last_receive_time > RX_TIMEOUT_MS) {
            stopReceive();
            LOG_DEBUG(F("Receive timeout"));
            flushOutput();
        } else {
            if(state->output_len >= 0.9*BUFFER_SIZE) {
                LOG_DEBUG(F("Buffer overflow"));
                flushOutput();
            }
            return;
        }
    }
    int16_t result = radio->scanChannel();
    if (result == CHANNEL_FREE) {
        //processInput();
    } else if (result == PREAMBLE_DETECTED) {
        startReceive();
    } else {
        LOG_WARN(F("Unexpected scan channel result:"), result);
    }
}

void Modem::serviceLoop() {
    if (!state->receive && state->input_len == 0) { // if not receving and transmitting
        transmitAdvertisementPacket();
        state->routing_table.cleanUpRoutes();
    }
}

void Modem::flushOutput() {
    if (state->output_len > 0) {
        send(state->output, state->output_len);
        memset(state->output, 0, BUFFER_SIZE);
        state->output_len = 0;
    }
}


void Modem::reset() {
    radio->reset();
    setup();
}

void Modem::transmit(uint8_t* data, size_t len) {
    Packet packet = {config()->address, state->address_destination};
    memcpy(packet.payload, data, len);
    memcpy(state->buffer, &packet, len + SERVICE_SIZE);
    LOG_DEBUG(F("Transmit"), len + SERVICE_SIZE);
    int16_t result = radio->transmit(state->buffer, len + SERVICE_SIZE);
    if (result != ERR_NONE) {
        error(F("Transmit"), result);
    }
    state->network.transmit += len;
    delay(TX_DELAY_MS);
}

void Modem::transmitAdvertisementPacket() {
    AdvertisementPacket packet = {{}, config()->address, config()->adv_period_millis};
    memcpy(packet.prefix, ADV_PREFIX, ADV_PREFIX_SIZE);
    memcpy(state->buffer, &packet, sizeof(AdvertisementPacket));
    radio->transmit(state->buffer, sizeof(AdvertisementPacket));
    LOG_DEBUG(F("Transmit adv packet"));
}

void Modem::receive() {
    if (state->is_received) {
        int64_t receive_time = millis();
        int16_t result = radio->readData(state->buffer, SX127X_MAX_PACKET_LENGTH);
        if (result == ERR_NONE) {
            size_t len = radio->getPacketLength();
            LOG_DEBUG(F("Received"), len, F("bytes in"), (int)(receive_time - state->last_receive_time), F("ms"));
            if (!receiveAdvertisementPacket(len)) {
                Packet packet = {};
                memcpy(&packet, state->buffer, len);
                if (packet.dst == config()->address || packet.dst == BROADCAST_ADDR) {
                    uint8_t payload_len = len - SERVICE_SIZE;
                    memcpy(state->output + state->output_len, packet.payload, payload_len);
                    state->output_len += payload_len;
                    state->network.receive += payload_len;
                    state->last_receive_time = receive_time;
                    flushOutput();
                    //LOG_VERBOSE(F("Processed in"), (int)(millis() - receive_time),  F("ms"));
                }
            } else {
                stopReceive();
            }
        } else {
            error(F("Receive"), result);
        }
        state->is_received = false;
        radio->startReceive();
    }
}

void Modem::startReceive() {
    state->last_receive_time = millis();
    state->receive = true;
    radio->setDio0Action([](){
        if (modem->state->receive) {
            modem->state->is_received = true;
        }
    });
    radio->startReceive(0, SX127X_RXCONTINUOUS);
}

void Modem::stopReceive() {
    state->receive = false;
    radio->clearDio0Action();
    LOG_DEBUG(F(""));
}

bool Modem::receiveAdvertisementPacket(size_t len) {
    if (len == sizeof(AdvertisementPacket)) {
        if (strncmp((char *)state->buffer, ADV_PREFIX, ADV_PREFIX_SIZE) == 0) {
            AdvertisementPacket packet = {};
            memcpy(&packet, state->buffer, sizeof(AdvertisementPacket));
            LOG_DEBUG(F("Receive adv packet. Address: "), packet.address, F(", period: "), packet.adv_period_millis);
            state->routing_table.addRoute(packet.address, packet.adv_period_millis, radio->getRSSI());
            return true;
        }
    }
    return false;
}

void Modem::error(const __FlashStringHelper* operation, int16_t code) {
    LOG_ERROR(operation, F("failed. code:"), code);
}