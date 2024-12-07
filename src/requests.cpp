#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5Unified.h>
#include <ArduinoWebsockets.h>
#include "requests.h"

ws::WebsocketsClient client;

String serverUrl;

void sendAudioRequest(
    int16_t* rec_data,
    size_t record_size
) {
          HTTPClient http;
        if (config.serverURL == "")
        {
            serverUrl = "http://192.168.1.108:1323/ConversationHandler";
        } else {
            serverUrl = config.serverURL + "/ConversationHandler";
        }
       
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/octet-stream");
        http.addHeader("X-User-ID", config.userID);
        http.addHeader("X-Device-ID", config.deviceID);
        http.addHeader("X-Language", config.language);

        // Prepare audio data
        int totalSize = record_size * sizeof(int16_t);

        uint8_t* audioData = (uint8_t*)rec_data;

        int httpResponseCode = http.POST(audioData, totalSize);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("HTTP Response code: " + String(httpResponseCode));
            Serial.println("Response: " + response);

            // Parse the JSON response and display the transcription (if needed)
            // Example:
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, response);
            String transcription = doc["transcription"];
            M5.Display.clear();
            M5.Display.setCursor(0, 0);
            M5.Display.print(transcription);
        }
        else {
            Serial.println("Error on sending POST: " + String(httpResponseCode));
        }

        http.end();

        return ;
}

void connectWebSocketIfNeeded() {
    if (!client.available()) {
        client.connect(config.serverURL + "/ws");
    }
}


void sendAudioPacketOverWebSocket(int16_t* data, size_t samples) {
    const size_t chunkSize = 512; // Number of samples per chunk
    size_t offset = 0;

    while (offset < samples) {
        size_t toSend = min(chunkSize, samples - offset);
        if (client.available()) {
            // Send binary data as little-endian
            client.sendBinary(reinterpret_cast<const char*>(&data[offset]), toSend * sizeof(int16_t));
            offset += toSend;
            // client.poll();
            delay(1); // Adjust delay as needed
        } else {
            // Reconnect or handle disconnection
            connectWebSocketIfNeeded();
            delay(100); // Wait before retrying
        }
    }
}

void sendWebsocketMessageIsOver() {
    if (client.available()) {
        client.send("EOS");
    } else {
        connectWebSocketIfNeeded();
        delay(100); // Wait before retrying
    }
}