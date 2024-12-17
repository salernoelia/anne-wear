#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5Unified.h>
#include <ArduinoWebsockets.h>
#include "requests.h"
#include "ui.h" 
#include <map>

#include <string>

namespace ws = websockets;
ws::WebsocketsClient client;
std::vector<Task> tasks; 

String serverUrl;

// --- WebSocket Connection States ---
typedef enum {
    WS_DISCONNECTED,
    WS_CONNECTING,
    WS_CONNECTED,
    WS_HANDSHAKE_SENT,
    WS_HANDSHAKE_RECEIVED,
    WS_ERROR
} WebSocketState;

WebSocketState wsState = WS_DISCONNECTED;

static bool websocketReady = false; // This might not be needed anymore if you rely solely on wsState
static unsigned long lastReconnectAttempt = 0;
static unsigned long lastPingSent = 0;
const unsigned long pingInterval = 30000; // Send a ping every 30 seconds

// --- Error States ---
typedef enum {
    ERROR_NONE = 0,
    ERROR_SERVER_UNREACHABLE,
    ERROR_WS_CONNECT_FAILED,
    ERROR_WS_HANDSHAKE_TIMEOUT,
    ERROR_WS_SEND_FAILED,
    ERROR_WS_RECEIVE_FAILED,
    ERROR_WS_CLOSED_UNEXPECTEDLY,
    // ... Add more error states as needed
} ErrorState;

ErrorState lastError = ERROR_NONE;

// --- Function to handle errors ---
void handleError(ErrorState error) {
    lastError = error;
    wsState = WS_ERROR;

    switch (error) {
        case ERROR_SERVER_UNREACHABLE:
            Serial.println("Error: Server unreachable.");
            displayErrorState("Server unreachable");
            break;
        case ERROR_WS_CONNECT_FAILED:
            Serial.println("Error: WebSocket connection failed.");
            displayErrorState("WS connect failed");
            break;
        case ERROR_WS_HANDSHAKE_TIMEOUT:
            Serial.println("Error: WebSocket handshake timeout.");
            displayErrorState("WS handshake timeout");
            break;
        case ERROR_WS_SEND_FAILED:
            Serial.println("Error: Failed to send data over WebSocket.");
            displayErrorState("WS send failed");
            break;
        case ERROR_WS_RECEIVE_FAILED:
            Serial.println("Error: Failed to receive data over WebSocket.");
            displayErrorState("WS receive failed");
            break;
        case ERROR_WS_CLOSED_UNEXPECTEDLY:
            Serial.println("Error: WebSocket closed unexpectedly.");
            displayErrorState("WS closed");
            break;
        // ... Handle other error states
        default:
            Serial.println("Error: Unknown error.");
            displayErrorState("Unknown error");
            break;
    }

    // Close the WebSocket connection to free resources
    client.close();
    wsState = WS_DISCONNECTED;

    // Add logic here to attempt recovery (e.g., retry connection after a delay) or halt the system
    // based on the error severity.
    // For now, we'll just reset the connection attempt timer to allow for reconnection attempts.
    lastReconnectAttempt = 0;
}

bool checkIfServerRespondsOK() {
    HTTPClient http;
    if (config.serverURL.isEmpty()) {
        serverUrl = "https://estation.space/ok";
    } else {
        serverUrl = config.serverURL + "/ok";
    }

    http.begin(serverUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) {
        Serial.println("Server is up.");
        http.end();
        return true;
    } else {
        Serial.print("Server is down. HTTP Response code: ");
        Serial.println(httpResponseCode);
        http.end();
        handleError(ERROR_SERVER_UNREACHABLE);
        return false;
    }
}

std::vector<Task>& getTasks() {
    return tasks;
}

bool getUserTasksThatAreStillDue() {
    HTTPClient http;
    String serverUrl;

    Serial.println("Fetching user tasks...");
    
    if (config.serverURL.isEmpty()) {
        serverUrl = "https://estation.space/tasks/" + config.userID;
    } else {
        serverUrl = config.serverURL + "/tasks/" + config.userID;
    }

    http.begin(serverUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Server response: " + payload);

        // Clear previous tasks
        tasks.clear();
        
        // Allocate JsonDocument
        DynamicJsonDocument doc(4096);
        
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print("Deserialize failed: ");
            Serial.println(error.c_str());
            http.end();
            return false;
        }

        JsonArray array = doc.as<JsonArray>();
        for (JsonVariant v : array) {
            Task task;
            task.ID = v["ID"].as<int64_t>();
            task.UserID = v["UserID"].as<String>();
            task.Title = v["Title"].as<String>();
            task.Description = v["Description"].as<String>();
            task.DueDate = v["DueDate"].as<String>();
            task.Completed = v["Completed"].as<bool>();
            task.CreatedAt = v["CreatedAt"].as<String>();
            
            // Debug output
            Serial.println("Parsed task:");
            Serial.println("ID: " + String(task.ID));
            Serial.println("Title: " + task.Title);
            Serial.println("Description: " + task.Description);
            
            tasks.push_back(task);
        }

        http.end();
        return true;
    } 
    
    http.end();
    handleError(ERROR_SERVER_UNREACHABLE);
    return false;
}

// Function to connect to WebSocket with custom headers and handle connection states
void connectWebSocketIfNeeded() {
    if (wsState == WS_CONNECTED || wsState == WS_CONNECTING) {
        return; 
    }

    unsigned long now = millis();
    if (now - lastReconnectAttempt < 5000) {
        return; 
    }
    lastReconnectAttempt = now;

    wsState = WS_CONNECTING;
    Serial.println("Attempting to connect to WebSocket...");

    if (client.connect(config.serverURL + "/ws")) {
        Serial.println("WebSocket connection initiated.");
        wsState = WS_HANDSHAKE_SENT;

        // Send headers JSON
        StaticJsonDocument<256> doc;
        doc["X-User-ID"] = config.userID;
        doc["X-Device-ID"] = config.deviceID;
        doc["X-Language"] = config.language;
        String headersJson;
        serializeJson(doc, headersJson);

        if (client.send(headersJson)) {
            Serial.println("Custom headers sent.");
        } else {
            Serial.println("Failed to send custom headers.");
            handleError(ERROR_WS_SEND_FAILED);
            client.close();
            wsState = WS_DISCONNECTED;
            return;
        }

        // Wait for server acknowledgment
        unsigned long start = millis();
        while (millis() - start < 3000) {
            client.poll();
            if (client.available()) {
                ws::WebsocketsMessage msg = client.readBlocking();
                if (msg.data() == "Headers received successfully.") {
                    wsState = WS_CONNECTED;
                    websocketReady = true;
                    Serial.println("Handshake complete.");
                    return;
                } else {
                    Serial.print("Received unexpected response: ");
                    Serial.println(msg.data());
                }
            }
            delay(50);
        }

        Serial.println("Handshake not acknowledged.");
        handleError(ERROR_WS_HANDSHAKE_TIMEOUT);
        client.close();
        wsState = WS_DISCONNECTED;

    } else {
        Serial.println("WebSocket connection failed.");
        handleError(ERROR_WS_CONNECT_FAILED);
        wsState = WS_DISCONNECTED;
    }
}

void sendPingWebSocket() {
    unsigned long now = millis();
    if (wsState == WS_CONNECTED && now - lastPingSent > pingInterval) {
        if (client.send("PING")) {
            Serial.println("Sent PING to server.");
            lastPingSent = now;
        } else {
            Serial.println("Failed to send PING.");
            handleError(ERROR_WS_SEND_FAILED);
            client.close(); 
            wsState = WS_DISCONNECTED;
        }
    } else if (wsState != WS_CONNECTED) {
        connectWebSocketIfNeeded();
    }
}

void handleWebSocketMessages() {
    if (client.available()) {
        ws::WebsocketsMessage msg = client.readBlocking();

        switch (msg.type()) { 
            case ws::MessageType::Text: 
                if (msg.data() == "PONG") {
                    Serial.println("Received PONG from server.");
                    lastPingSent = millis();
                } else if (msg.data() == "celebration") {
                    currentEmotion = "celebration";
                } else {
                    Serial.print("Received text: ");
                    Serial.println(msg.data());
                }
                break;

            case ws::MessageType::Binary:
                Serial.print("Received binary data, length: ");
                Serial.println(msg.length());
                break;

            case ws::MessageType::Close:
                Serial.println("Received close message from server.");
                client.close();
                wsState = WS_DISCONNECTED;
                handleError(ERROR_WS_CLOSED_UNEXPECTEDLY);
                break;

            case ws::MessageType::Ping:
                Serial.println("Received ping from server, sending pong.");
                if (!client.send("PONG")) {
                    Serial.println("Failed to send PONG.");
                }
                break;

            case ws::MessageType::Pong:
                Serial.println("Received pong from server.");
                break;

            default:
                Serial.print("Received unknown message type: ");
                Serial.println(static_cast<int>(msg.type()));
                break;
        }
    }
}

void sendAudioPacketOverWebSocket(int16_t* data, size_t samples) {
    if (wsState != WS_CONNECTED) {
        connectWebSocketIfNeeded();
        if (wsState != WS_CONNECTED) {
            Serial.println("WebSocket not connected. Unable to send audio data.");
            return;
        }
    }

    const size_t chunkSize = 512;
    size_t offset = 0;
    while (offset < samples) {
        size_t toSend = min(chunkSize, samples - offset);
        if (client.sendBinary(reinterpret_cast<const char*>(&data[offset]), toSend * sizeof(int16_t))) {
            offset += toSend;
        } else {
            Serial.println("Failed to send audio packet.");
            handleError(ERROR_WS_SEND_FAILED);
            client.close();
            wsState = WS_DISCONNECTED;
            return;
        }
        delay(1);
    }
}

void sendWebsocketMessageIsOver() {
    if (wsState == WS_CONNECTED) {
        if (!client.send("EOS")) {
            Serial.println("Failed to send EOS message.");
            handleError(ERROR_WS_SEND_FAILED);
            client.close();
            wsState = WS_DISCONNECTED;
        }
    } else {
        connectWebSocketIfNeeded();
        if (wsState == WS_CONNECTED) {
            if (!client.send("EOS")) {
                Serial.println("Failed to send EOS message.");
                handleError(ERROR_WS_SEND_FAILED);
                client.close();
                wsState = WS_DISCONNECTED;
            }
        }
    }
}