#include <SPIFFS.h>
#include <string.h>
#include <WiFi.h>
#include "server.h"
#include "message.h"

const char* SSID_PREFIX = "LoRaModem";
const char* AP_HOSTNAME = "loramodem.online";
IPAddress ip;

bool onValidHost(AsyncWebServerRequest *request) {
    return request->host().equals(AP_HOSTNAME) || request->host().equals(ip.toString());
}

bool onWrongHost(AsyncWebServerRequest *request) {
    return !onValidHost(request);
}

char* getHostname(ModemConfig* config) {
    char* hostname = new char[strlen(SSID_PREFIX) + 6];
    sprintf(hostname, "%s_%04X", SSID_PREFIX, config->address);
    return hostname;
}

char* getSSID(ModemConfig* config) {
    if (strlen(config->network.ssid) == 0) {
        return getHostname(config);
    } else { 
        return config->network.ssid;
    }
}

uint32_t configureWiFi(ModemConfig* config, ModemDisplay* display) {
    switch (config->network.WIFI_mode) {
        case OFF:
            ESP_LOGD(TAG, "OFF");
            break;
        case AP: {
            ESP_LOGD(TAG, "AP");
            char* ssid = getSSID(config);
            ESP_LOGD(TAG, "SSID: %s", ssid);
            char* password = config->network.password;
            bool result = WiFi.softAP(ssid, password, config->network.channel);
            if (result) {
                IPAddress result = WiFi.softAPIP();
                display->apInfo(ssid);
                return (uint32_t)result;
            } else {
                ESP_LOGE(TAG, "WiFi AP error");
                display->message("WiFi AP error");
            }
            break;
        }
        case STA: {
            ESP_LOGD(TAG, "STA");
            char* ssid = getSSID(config);
            ESP_LOGD(TAG, "SSID: %s", ssid);
            char* password = config->network.password;
            WiFi.mode(WIFI_STA);
            WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
            WiFi.setHostname(getHostname(config));
            WiFi.begin(ssid, password);
            int8_t connect_counter = 0;
            char message[64];
            sprintf(message, "Connecting to\n%s", ssid);
            display->message(message);
            while (WiFi.status() != WL_CONNECTED && connect_counter++ <= 100) {
                delay(100);
                display->progress(connect_counter);
            }
            if (WiFi.status() == WL_CONNECTED) {
                ESP_LOGD(TAG, "Connected");
                display->progress(100);
                IPAddress ip = WiFi.localIP();
                display->staInfo(ip.toString(), WiFi.RSSI());
                return (uint32_t)ip;
            } {
                sprintf(message, "Connection to\n%s failed", ssid);
                ESP_LOGE(TAG, "%s", message);
                display->message(message);
                display->showNetworkStat(0, 0);
            }
        }
    }
    WiFi.mode(WIFI_MODE_NULL);
    return 0;
}

ModemServer::ModemServer(Modem* m, ModemDisplay* d, void* q): modem(m), display(d), messageQueue(q) { };

void ModemServer::setup() {
    if(!SPIFFS.begin(true)){
        ESP_LOGE(TAG, "SPIFFS error");
        return;
    }
    uint32_t ip_num = configureWiFi(modem->persister->getConfig(), display);
    if (ip_num == 0) {
        return;
    }
    ip = IPAddress(ip_num);
    modem->state->network.ip = ip_num;
    ESP_LOGD(TAG, "IP: %s", ip.toString());
    mode = modem->persister->getConfig()->network.WIFI_mode;
    if (mode == AP) {
        dnsServer.start(53, "*", ip);
        server->addHandler(new AnyURLRequestHandler()).setFilter(onWrongHost);
    }
    ws->onEvent([this](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
        onEvent(server, client, type, arg, data, len);
    });
    server->addHandler(ws);
    server->serveStatic("/", SPIFFS, "").setDefaultFile("index.html").setFilter(onValidHost);
    server->begin();
    ESP_LOGI(TAG, "Server started");
}

void ModemServer::onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_DATA) {
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            Message message;
            uint32_t id = client->id();
            message.client_id = &id;
            message.len = Serial.readBytes(message.data, PAYLOAD_SIZE);
            message.to_transmit = true;
            xQueueSend(messageQueue, (void *) &message, 100);
            //modem->setInput(data, len);
        } else {
            client->text("Too long");
        }
    }
}

void ModemServer::send(uint8_t* data, size_t len) {
    if (mode != OFF) {
        ESP_LOGD(TAG, String((char*) data));
        ws->binaryAll(data, len);
    }
}

void ModemServer::loop() {
    if (mode == AP) {
        dnsServer.processNextRequest();
    }
}

void ModemServer::serviceLoop() {
    if (mode != OFF) {
        ws->cleanupClients();
        display->updateClients(ws->count());
        if (mode == STA) {
            int8_t rssi = WiFi.RSSI();
            modem->state->network.rssi = rssi;
            display->updateWifiLevel(WiFi.RSSI());
        }
    }
}

bool AnyURLRequestHandler::canHandle(AsyncWebServerRequest *request){
    return true;
}

void AnyURLRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    ESP_LOGD(TAG, "%s->%s", request->host(), AP_HOSTNAME);
    char url[strlen(AP_HOSTNAME) + 8];
    sprintf(url, "http://%s", AP_HOSTNAME);
    request->redirect(String(url));
}
