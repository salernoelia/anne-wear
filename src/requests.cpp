#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5Unified.h>
#include <ArduinoWebsockets.h>
#include "requests.h"
#include <map>
#include <string>

ws::WebsocketsClient client;

String serverUrl;



bool checkIfServerRespondsOK() {
    HTTPClient http;
    if (config.serverURL == "")
    {
        serverUrl = "https://estation.space/ok";
    } else {
        serverUrl = config.serverURL + "/ok";
    }

    http.begin(serverUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
        Serial.println("Server is up.");
        http.end();
        return true;
    } else {
        Serial.println("Server is down.");
        http.end();
        return false;
    }
}


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
            DeserializationError error = deserializeJson(doc, response);
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                // Handle the error accordingly
                return;
            }

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

// Function to connect to WebSocket with custom headers
void connectWebSocketIfNeeded() {
    if (!client.available()) {
        // Attempt to connect to the WebSocket server
        if (client.connect(config.serverURL + "/ws")) {
            Serial.println("WebSocket connection initiated.");

            // Create a JSON object with the custom headers
            StaticJsonDocument<256> doc;
            doc["X-User-ID"] = config.userID;
            doc["X-Device-ID"] = config.deviceID;
            doc["X-Language"] = config.language;

            // Serialize JSON to a string
            String headersJson;
            serializeJson(doc, headersJson);

            client.onMessage([](ws::WebsocketsClient &c, ws::WebsocketsMessage message) {
                Serial.println("Received WebSocket message:");
                Serial.println(message.data());
                // turn string into WAV file and play 
                // M5.Speaker.playWav((uint8_t*)message.data().c_str());
            });


            // Send the headers as a text message
            client.send(headersJson);
            Serial.println("Custom headers sent as JSON string.");

            // Optionally, wait for a short period to ensure the message is sent
            delay(10);
        } else {
            Serial.println("WebSocket connection failed.");
            return;
        }
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
            break; 
        }
    }
}

void sendWebsocketMessageIsOver() {
    if (client.available()) {
        client.send("EOS");
    } else {
        connectWebSocketIfNeeded();
        delay(100); // Wait before retrying

        if (client.available()) {
            client.send("EOS");
        } else {
            Serial.println("Failed to send EOS message.");
            return;
        }
    }
}