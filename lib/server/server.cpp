#include <SPIFFS.h>
#include <string.h>
#include <WiFi.h>
#include "server.h"
#include "message.h"
#include "config.h"

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

void dns_task(void *pvParameter){
    DNSServer* dnsServer = (DNSServer*) pvParameter;
    while(dnsServer) {
        dnsServer->processNextRequest();
        vTaskDelay(10 / portTICK_RATE_MS);
    }
    vTaskDelete( NULL );
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
    WiFiMode mode = modem->persister->getConfig()->network.WIFI_mode;
    if (mode == AP) {
        dnsServer.start(53, "*", ip);
        server->addHandler(new AnyURLRequestHandler()).setFilter(onWrongHost);
        xTaskCreatePinnedToCore(&dns_task, "dns", 2048, &dnsServer, 1, NULL, 0);
        ESP_LOGI(TAG, "DNS started");
    }
    ws->onEvent([this](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
        onEvent(server, client, type, arg, data, len);
    });
    server->addHandler(ws);
    server->serveStatic("/", SPIFFS, "").setDefaultFile("index.html").setFilter(onValidHost);
    server->begin();
    is_started = true;
    ESP_LOGI(TAG, "Server started");
}

uint32_t ModemServer::clients() {
    ws->cleanupClients();
    return ws->count();
}


bool ModemServer::isStarted() {
    return is_started;
}

void ModemServer::onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    ESP_LOGD(TAG, "type: %d", type);
    if(type == WS_EVT_DATA) {
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            Message message;
            uint32_t id = client->id();
            message.client_id = &id;
            message.len = len;
            message.to_transmit = true;
            memcpy(message.data, data, len);
            xQueueSend(messageQueue, (void *) &message, 100);
        } else {
            client->text("Too long");
        }
    }
}

void ModemServer::send(uint32_t* id, uint8_t* data, size_t len) {
    if (is_started) {
        if (id == nullptr) {
            ESP_LOGD(TAG, "all:%s", (char*) data);
            ws->binaryAll(data, len);
        } else {
            ESP_LOGD(TAG, "%u:%s", *id, (char*) data);
            ws->binary(*id, data, len);
        }
    }
}

bool AnyURLRequestHandler::canHandle(AsyncWebServerRequest *request) {
    return true;
}

void AnyURLRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    ESP_LOGD(TAG, "%s->%s", request->host(), AP_HOSTNAME);
    char url[strlen(AP_HOSTNAME) + 8];
    sprintf(url, "http://%s", AP_HOSTNAME);
    request->redirect(String(url));
}
