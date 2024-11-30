// main.cpp
#include <Arduino.h>
#include <M5Unified.h>
#include "config.h"
#include <WiFi.h>
#include "wifisetup.h"



// Variables to track WiFi status
unsigned long previousWiFiCheck = 0;
bool lastWiFiStatus = false; // false: not connected, true: connected

void setup(void)
{
    // Initialize Serial for debugging
    Serial.begin(9600);
    while (!Serial); // Wait for Serial to initialize (optional)

    // Initialize M5Unified with default configuration
    auto cfg = M5.config();
    M5.begin(cfg);

    // Load configuration
    if (!loadConfig()) {
        Serial.println("Failed to load configuration. Using default settings.");
    }

    // Initialize WiFi
    bool connected = initWiFi();
    lastWiFiStatus = connected;
}

void loop() {
    M5.update(); // Handle background tasks

    checkConnectionStatus(lastWiFiStatus, previousWiFiCheck);

   

    delay(100);
}
