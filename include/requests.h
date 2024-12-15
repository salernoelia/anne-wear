#ifndef REQUESTS_H
#define REQUESTS_H
#include <ArduinoWebsockets.h>

namespace ws = websockets;
extern ws::WebsocketsClient client;

#include "config.h"    // Include config.h to ensure config is available
#include <HTTPClient.h> // HTTPClient library for making HTTP requests
#include <ArduinoJson.h> // ArduinoJson for handling JSON data
#include <M5Unified.h>   // M5Unified for M5Stack display



bool checkIfServerRespondsOK();
void connectWebSocketIfNeeded();
void sendPingWebSocket();

void sendAudioPacketOverWebSocket(
    int16_t* rec_data,
    size_t record_size
);

void sendWebsocketMessageIsOver();
void handleWebSocketMessage(websockets::WebsocketsMessage message);

#endif // REQUESTS_H
