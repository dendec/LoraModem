#ifndef SERVER_H
#define SERVER_H
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "display.h"
#include "modem.h"

class ModemServer {
    public:
        ModemServer(Modem* m, ModemDisplay* d, void* queue);
        void setup();
        uint32_t clients();
        void send(uint32_t* id, uint8_t* data, size_t len);
        bool isStarted();
    private:
        DNSServer dnsServer;
        Modem* modem;
        ModemDisplay* display;
        xQueueHandle messageQueue;
        AsyncWebServer* server = new AsyncWebServer(80);
        AsyncWebSocket* ws =  new AsyncWebSocket("/ws");
        bool is_started = false;
        void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
};

class AnyURLRequestHandler: public AsyncWebHandler {
    public:
        bool canHandle(AsyncWebServerRequest *request);
        void handleRequest(AsyncWebServerRequest *request);
};

#endif