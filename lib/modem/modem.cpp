#include "config.h"
#include "modem.h"

extern Modem* modem;
extern SemaphoreHandle_t modem_semaphore;

Modem::Modem(SX1278* r): radio(r) {
    persister = new ConfigPersister();
    state = new ModemState();
}

void Modem::setup() {
    int16_t err = radio->begin(
        persister->getConfig()->radio.frequency,
        persister->getConfig()->radio.bandwidth,
        persister->getConfig()->radio.sfactor,
        persister->getConfig()->radio.coding_rate,
        SX127X_SYNC_WORD, 
        persister->getConfig()->radio.power,
        16); 
    if (isImplicitHeader()) {
        ESP_LOGI(TAG, "Implicit header is used");
        radio->implicitHeader(PACKET_SIZE);
    } else {
        radio->explicitHeader();
    }
    radio->setCRC(persister->getConfig()->radio.crc);
    if (err != ERR_NONE) {
        ESP_LOGE(TAG, "LoRa initialization failed. code: %d", err);
        persister->reset();
        setup();
    } else {
        ESP_LOGI(TAG, "LoRa initialized");
        receive();
    }
}

void Modem::reset() {
    radio->reset();
    setup();
}

void Modem::transmitPacket(uint8_t* data, size_t len) {
    Packet packet = {persister->getConfig()->address, state->address_destination};
    uint8_t packetSize;
    if (isImplicitHeader() && len < PAYLOAD_SIZE) {
        memset(packet.payload, 0, PAYLOAD_SIZE);
        packetSize = PACKET_SIZE - 1;
    } else {
        packetSize = len + SERVICE_SIZE;
    }
    memcpy(packet.payload, data, len);
    transmit((uint8_t*)&packet, packetSize);
    state->network.transmit += len;
}

void Modem::transmitAdvertisingPacket() {
    Packet packet = {persister->getConfig()->address, BROADCAST_ADDR, {}};
    if (isImplicitHeader()) {
        memset(packet.payload, 0, PAYLOAD_SIZE);
        transmit((uint8_t*)&packet, sizeof(Packet));
    } else {
        transmit((uint8_t*)&packet, SERVICE_SIZE);
    }
    ESP_LOGI(TAG, "Service packet sent");
}

ICACHE_RAM_ATTR
void modemISR(void) {
    static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR( modem_semaphore, &xHigherPriorityTaskWoken );
    if( xHigherPriorityTaskWoken == pdTRUE )
    {
        portYIELD_FROM_ISR();
    }
}

void Modem::transmit(uint8_t* data, size_t len) {
    state->receiving = false;
    ESP_LOGV(TAG, "%s", (char*)data);
    radio->setDio0Action(modemISR);
    int16_t result = radio->startTransmit(data, len);
    if (result != ERR_NONE) {
        ESP_LOGE(TAG, "Transmit failed. code: %d", result);
    }
}

void Modem::receive() {
    state->receiving = true;
    radio->setDio0Action(modemISR);
    if (isImplicitHeader()) {
        radio->startReceive(PACKET_SIZE, SX127X_RXCONTINUOUS);
    } else {
        radio->startReceive(0, SX127X_RXCONTINUOUS);
    }
    
}

bool Modem::isImplicitHeader() {
    return persister->getConfig()->radio.sfactor == 6;
}