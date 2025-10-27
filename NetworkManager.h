#pragma once
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class NetworkManager {
public:
    NetworkManager(int port = 3333);
    void begin(const char* ssid, const char* password);
    void setCommandCallback(std::function<void(String)> callback);
    void sendResponse(const String& response);
    String getLocalIP() const;
    bool isConnected() const;
    
private:
    AsyncServer* server;
    AsyncClient* currentClient;
    std::function<void(String)> commandCallback;
    int serverPort;
    
    void handleNewClient(AsyncClient* client);
    void handleClientData(AsyncClient* client, void* data, size_t len);
    void handleClientDisconnect(AsyncClient* client);
};
