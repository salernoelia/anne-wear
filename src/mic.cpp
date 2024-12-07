// mic.cpp
#include <M5Unified.h>
#include <Arduino.h>
#include <WiFi.h>
#include "mic.h"
#include <ArduinoJson.h>
#include "config.h"
#include "requests.h"
#include "ui.h"

// Audio recording variables
int16_t prev_y[record_length];
int16_t prev_h[record_length];
size_t rec_record_idx = 2;
size_t draw_record_idx = 0;

int16_t* rec_data;
size_t current_samples_collected = 0; 

// State variable to track recording status
bool isRecording = false;

void initMic()
{
    // Initialize the microphone
    M5.Mic.begin();

    rec_data = (int16_t*)heap_caps_malloc(record_size * sizeof(int16_t), MALLOC_CAP_8BIT);
    if (rec_data == NULL) {
        Serial.println("Failed to allocate memory for rec_data");

    }

    memset(rec_data, 0 , record_size * sizeof(int16_t));
    M5.Speaker.setVolume(255);

}

void updateMic()
{
    // Check if Button A is being held
    if (M5.BtnA.isHolding())
    {
        M5.Display.clear();
        if (!isRecording)
        {
            // Start recording
            Serial.println("Button A pressed. Starting recording...");
            isRecording = true;
            memset(rec_data, 0, record_size * sizeof(int16_t));
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
               animateAudioWave(
                     data, 
                     record_length, 
                     record_samplerate, 
                     record_number, 
                     shift, 
                     prev_y, 
                     prev_h, 
                     draw_record_idx, 
                     rec_record_idx, 
                     rec_data
               );
        
            sendAudioPacketOverWebSocket(data, record_length);

            current_samples_collected += record_length;

            if (++rec_record_idx >= record_number) { rec_record_idx = 0; }
            if (++draw_record_idx >= record_number) { draw_record_idx = 0; }

            memset(rec_data, 0 , record_size * sizeof(int16_t));
            }
            else 
            {
            Serial.println("Recording failed");
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
        displayHomeScreen();
    }
}

void sendAudioData()
{
    // Stop recording
    while (M5.Mic.isRecording()) { M5.delay(1); }
    M5.Mic.end();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Sending audio data...");
        Serial.println("Record size: " + String(record_size));

         if (current_samples_collected > 0) {
                //sendAudioRequest(rec_data, record_size);
                sendWebsocketMessageIsOver();
                Serial.println("Sending audio data...");
                memset(rec_data, 0, record_size * sizeof(int16_t));
                current_samples_collected = 0;
            }
        

  
    }
    else
    {
        Serial.println("Wi-Fi not connected");
    }

    // Restart the microphone
    M5.Mic.begin();
}
