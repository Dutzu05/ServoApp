#include "NetworkManager.h"

NetworkManager::NetworkManager(int port) : server(nullptr), currentClient(nullptr), serverPort(port) {}

void NetworkManager::begin(const char* ssid, const char* password) {
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Start TCP server
    server = new AsyncServer(serverPort);
    
    server->onClient([this](void* arg, AsyncClient* client) {
        handleNewClient(client);
    }, nullptr);
    
    server->begin();
    Serial.println("TCP Server started on port " + String(serverPort));
}

void NetworkManager::handleNewClient(AsyncClient* client) {
    Serial.println("New client connected: " + client->remoteIP().toString());
    currentClient = client;
    
    client->onData([this](void* arg, AsyncClient* client, void* data, size_t len) {
        handleClientData(client, data, len);
    }, nullptr);
    
    client->onDisconnect([this](void* arg, AsyncClient* client) {
        handleClientDisconnect(client);
    }, nullptr);
}

void NetworkManager::handleClientData(AsyncClient* client, void* data, size_t len) {
    String command = String((char*)data).substring(0, len);
    command.trim();
    
    Serial.println("Received command: " + command);
    
    if (commandCallback) {
        commandCallback(command);
    }
}

void NetworkManager::handleClientDisconnect(AsyncClient* client) {
    Serial.println("Client disconnected: " + client->remoteIP().toString());
    if (currentClient == client) {
        currentClient = nullptr;
    }
}

void NetworkManager::setCommandCallback(std::function<void(String)> callback) {
    commandCallback = callback;
}

void NetworkManager::sendResponse(const String& response) {
    if (currentClient && currentClient->connected()) {
        currentClient->write(response.c_str());
    }
}

String NetworkManager::getLocalIP() const {
    return WiFi.localIP().toString();
}

bool NetworkManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}
