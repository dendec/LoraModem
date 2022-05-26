#include "config.h"
#include "modem.h"

const uint8_t ADV_PREFIX_SIZE = 13;
const char* ADV_PREFIX = "ADVERTISEMENT";
extern Modem* modem;
extern SemaphoreHandle_t modem_semaphore;

struct AdvertisementPacket {
    char prefix[ADV_PREFIX_SIZE];
    uint16_t address;
    uint16_t adv_period_millis;
};

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
        persister->getConfig()->radio.preamble,
        persister->getConfig()->radio.gain);
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
    memcpy(packet.payload, data, len);
    transmit((uint8_t*)&packet, len + SERVICE_SIZE);
    state->network.transmit += len;
}

void Modem::transmitAdvertisementPacket() {
    AdvertisementPacket packet = {{}, persister->getConfig()->address, persister->getConfig()->adv_period_millis};
    memcpy(packet.prefix, ADV_PREFIX, ADV_PREFIX_SIZE);
    transmit((uint8_t*)&packet, sizeof(AdvertisementPacket));
    ESP_LOGI(TAG, "Adv packet sent");
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
    radio->startReceive(0, SX127X_RXCONTINUOUS);
}

bool Modem::receiveAdvertisementPacket(uint8_t* buffer, size_t len) {
    if (len == sizeof(AdvertisementPacket)) {
        if (strncmp((char *)buffer, ADV_PREFIX, ADV_PREFIX_SIZE) == 0) {
            AdvertisementPacket packet = {};
            memcpy(&packet, buffer, sizeof(AdvertisementPacket));
            ESP_LOGI(TAG, "Received adv packet. Address: %04X, period: %d", packet.address, packet.adv_period_millis);
            state->routing_table.addRoute(packet.address, packet.adv_period_millis, radio->getRSSI());
            return true;
        }
    }
    return false;
}