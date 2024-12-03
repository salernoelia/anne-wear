// main.cpp
#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "mic.h"
#include "config.h"
#include "wifisetup.h"


// Variables to track Wi-Fi status
unsigned long previousWiFiCheck = 0;
bool lastWiFiStatus = false; // false: not connected, true: connected

void setup(void)
{
    // Initialize Serial for debugging
    Serial.begin(9600);
    while (!Serial); // Wait for Serial to initialize

    // Initialize M5Unified with default configuration
    auto cfg = M5.config();
    M5.begin(cfg);

    if (!loadConfig()) {
        Serial.println("Failed to load configuration. Using default settings.");
    };


    bool connected = initWiFi();
    lastWiFiStatus = connected;

    // Initialize display
    M5.Display.startWrite();
    if (M5.Display.width() > M5.Display.height())
    {
        M5.Display.setRotation(M5.Display.getRotation() ^ 1);
    }
    M5.Display.setCursor(0, 0);
    M5.Display.print("REC\n");

    // Allocate memory for recording data
    rec_data = (int16_t*)heap_caps_malloc(record_size * sizeof(int16_t), MALLOC_CAP_8BIT);
    memset(rec_data, 0, record_size * sizeof(int16_t));
    M5.Speaker.setVolume(255);

    // Turn off the speaker to use the microphone
    M5.Speaker.end();
    M5.Mic.begin();
}

void loop() {
    M5.update(); // Handle background tasks

    checkConnectionStatus(lastWiFiStatus, previousWiFiCheck);

    // Update microphone and recording
    updateMic();

    delay(10);
}
