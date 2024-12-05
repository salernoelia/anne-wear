// mic.cpp

#include <M5UnitLCD.h>
#include <M5UnitOLED.h>
#include <M5Unified.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "mic.h"
#include <ArduinoJson.h>
#include "config.h"

// Audio recording variables
int16_t prev_y[record_length];
int16_t prev_h[record_length];
size_t rec_record_idx = 2;
size_t draw_record_idx = 0;

int16_t* rec_data;

String serverUrl;

// State variable to track recording status
bool isRecording = false;

void updateMic()
{
    // Check if Button A is being held
    if (M5.BtnA.isHolding())
    {
        if (!isRecording)
        {
            // Start recording
            Serial.println("Button A pressed. Starting recording...");
            isRecording = true;
            memset(rec_data, 0, record_size * sizeof(int16_t)); // Clear previous data
            rec_record_idx = 0;
            draw_record_idx = 0;
        }

        // Record audio data
        if (M5.Mic.isEnabled())
        {
            static constexpr int shift = 6;
            auto data = &rec_data[rec_record_idx * record_length];
            if (M5.Mic.record(data, record_length, record_samplerate))
            {
                data = &rec_data[draw_record_idx * record_length];

                int32_t w = M5.Display.width();
                if (w > record_length - 1) { w = record_length - 1; }
                for (int32_t x = 0; x < w; ++x)
                {
                    M5.Display.writeFastVLine(x, prev_y[x], prev_h[x], TFT_BLACK);
                    int32_t y1 = (data[x] >> shift);
                    int32_t y2 = (data[x + 1] >> shift);
                    if (y1 > y2)
                    {
                        int32_t tmp = y1;
                        y1 = y2;
                        y2 = tmp;
                    }
                    int32_t y = (M5.Display.height() >> 1) + y1;
                    int32_t h = (M5.Display.height() >> 1) + y2 + 1 - y;
                    prev_y[x] = y;
                    prev_h[x] = h;
                    M5.Display.writeFastVLine(x, y, h, TFT_WHITE);
                }
                M5.Display.display();

                if (++draw_record_idx >= record_number) { draw_record_idx = 0; }
                if (++rec_record_idx >= record_number) { rec_record_idx = 0; }
            }
        }
    }
    else
    {
        if (isRecording)
        {
            // Stop recording
            Serial.println("Button A released. Stopping recording and sending data...");
            isRecording = false;
            sendAudioData();
        }
    }
}

void sendAudioData()
{
    // Stop recording
    while (M5.Mic.isRecording()) { M5.delay(1); }
    M5.Mic.end(); // Stop the microphone to free up resources

    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        if (config.serverURL == "")
        {
            serverUrl = "http://192.168.1.108:1323/ConversationHandler";
        } else {
            serverUrl = config.serverURL;
        }
       
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/octet-stream");
        http.addHeader("X-User-ID", config.userID);
        http.addHeader("X-Device-ID", config.deviceID);

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
    }
    else
    {
        Serial.println("Wi-Fi not connected");
    }

    // Restart the microphone
    M5.Mic.begin();
}
