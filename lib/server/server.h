#ifndef SERVER_H
#define SERVER_H
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "display.h"
#include "modem.h"

class ModemServer {
    public:
        ModemServer(Modem* m, ModemDisplay* d);
        void setup();
        void restart();
        void loop();
        void serviceLoop();
        void send(uint8_t* data, size_t len);
    private:
        DNSServer dnsServer;
        Modem* modem;
        ModemDisplay* display;
        AsyncWebServer* server = new AsyncWebServer(80);
        AsyncWebSocket* ws =  new AsyncWebSocket("/ws");
        WiFiMode mode = OFF;
        void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
};

class AnyURLRequestHandler: public AsyncWebHandler {
    public:
        bool canHandle(AsyncWebServerRequest *request);
        void handleRequest(AsyncWebServerRequest *request);
};

#endif